#include "RenderContext.h"
#include "main.h"
#include <QOpenGLFunctions>
#include <QDebug>
#include <QThread>
/*static*/ QString RenderContext::defaultVertexShaderSource()
{
    return QString(
    "#version 130\n"
    "out vec2 uv;\n"
    "void main() {\n"
    "    vec2 _uv;\n"
    "    switch(gl_VertexID) {\n"
    "    case 0: _uv = vec2(1.,1.);break;\n"
    "    case 1: _uv = vec2(1.,-1.);break;\n"
    "    case 2: _uv = vec2(-1.,1.);break;\n"
    "    case 3: _uv = vec2(-1.,-1.);break;\n"
    "    default:_uv = vec2(0.,0.);break;\n"
    "    }\n"
    "    gl_Position = vec4(_uv,0.,1.);\n"
    "    uv = (_uv + vec2(1.,1.)) * 0.5;\n"
    "}");

}
/*static*/ QOpenGLShader *RenderContext::defaultVertexShader()
{
    static QOpenGLShader vert(QOpenGLShader::Vertex);
//c    if(!vert.isCompiled())
    vert.compileSourceCode(defaultVertexShaderSource());
    if(!vert.isCompiled()) {
        qDebug() << vert.log();
    }
    return &vert;
}
/*static*/ QOpenGLShaderProgram  *RenderContext::defaultVertexHalf()
{
    auto res = std::make_unique<QOpenGLShaderProgram>();
    res->addShaderFromSourceCode(QOpenGLShader::Vertex,
    defaultVertexShaderSource());
    return res.release();
}
RenderContext::RenderContext()
    : context(nullptr)
    , surface(nullptr)
    , timer(nullptr)
    , m_premultiply(nullptr)
    , m_outputCount(2)
    , m_currentSyncSource(NULL)
    , m_rendering(2)
    , m_noiseTextures(m_outputCount)
    , m_blankFbo()
    , m_framePeriodLPF(0)
{
    connect(this, &RenderContext::addVideoNodeRequested, this, &RenderContext::addVideoNode, Qt::QueuedConnection);
    connect(this, &RenderContext::removeVideoNodeRequested, this, &RenderContext::removeVideoNode, Qt::QueuedConnection);
    connect(this, &RenderContext::renderRequested, this, &RenderContext::render, Qt::QueuedConnection);
}

RenderContext::~RenderContext() {
    m_premultiply = 0;
    m_noiseTextures.clear();
    surface = 0;
    context->deleteLater();
    context = 0;
}

void RenderContext::start() {
    qDebug() << "Calling start from" << QThread::currentThread();
    context = new QOpenGLContext(this);
    {
        auto fmt = QSurfaceFormat::defaultFormat();
        if(auto scontext = QOpenGLContext::globalShareContext()) {
            fmt = scontext->format();
            context->setShareContext(scontext);
        }
//        fmt.setVersion(4,5);
        fmt.setVersion(3,0);
        fmt.setAlphaBufferSize(8);
        fmt.setRedBufferSize(8);
        fmt.setGreenBufferSize(8);
        fmt.setBlueBufferSize(8);
        fmt.setStencilBufferSize(8);
        fmt.setDepthBufferSize(24);
        context->setFormat(fmt);
        if(!context->create()) {
            throw std::runtime_error("FUUUUUUUUUCK");
        }
        qDebug() << "Desired format\n\n" << fmt << "\n\n";
        qDebug() << "Actual  format\n\n" << context->format()<< "\n\n";
    }

    // Creating a QOffscreenSurface with no window
    // may fail on some platforms
    // (e.g. wayland)
    {
        auto tmp_surface = QSharedPointer<QOffscreenSurface>(
            new QOffscreenSurface(),
               &QOffscreenSurface::deleteLater
                );
        tmp_surface->setFormat(context->format());
        tmp_surface->create();
        surface = tmp_surface;
    }
    elapsed_timer.start();
}

void RenderContext::checkLoadShaders() {
    auto program = m_premultiply;
    if(program)
        return;

    program.reset(defaultVertexHalf());
    program->addShaderFromSourceCode(QOpenGLShader::Fragment,
                                       "#version 130\n"
                                       "in vec2 uv;\n"
                                       "uniform sampler2D iFrame;\n"
                                       "out vec4 fragColor;\n"
                                       "void main() {\n"
                                        "    vec4 l = texture2D(iFrame, uv);\n"
                                       "    fragColor = vec4(l.rgb * l.a, l.a);\n"
                                       "}\n");
    if(!program->link())
        throw std::runtime_error("FUUUUUUUUUCK");
    m_premultiply.swap(program);
}

SharedTexPointer RenderContext::noiseTexture(int i) const {
    return m_noiseTextures.at(i);
}
SharedFboPointer RenderContext::blankFbo() const {
    return m_blankFbo;
}


void RenderContext::update() {
    if(m_rendering.tryAcquire())
        emit renderRequested();
}

void RenderContext::checkCreateNoise() {
    for(int i=0; i<m_outputCount; i++) {
        auto tex = m_noiseTextures.at(i);
        if(tex &&
           tex->width() == fboSize(i).width() &&
           tex->height() == fboSize(i).height()) {
            continue;
        }
        tex = SharedTexPointer::create(QOpenGLTexture::Target2D);
        tex->setSize(fboSize(i).width(), fboSize(i).height());
        tex->setFormat(QOpenGLTexture::RGBA8_UNorm);
        tex->allocateStorage(QOpenGLTexture::RGBA, QOpenGLTexture::UInt8);
        tex->setMinMagFilters(QOpenGLTexture::Linear, QOpenGLTexture::Linear);
        tex->setWrapMode(QOpenGLTexture::Repeat);

        auto byteCount = fboSize(i).width() * fboSize(i).height() * 4;
        auto data = std::make_unique<uint8_t[]>(byteCount);
        qsrand(1);
        std::generate(&data[0],&data[0] + byteCount,qrand);
        tex->setData(QOpenGLTexture::RGBA, QOpenGLTexture::UInt8, &data[0]);
        m_noiseTextures[i] = tex;
    }
}

void RenderContext::checkCreateBlankFbo()
{
    auto blank = m_blankFbo;
    if(!blank) {
        blank = SharedFboPointer::create(QSize(1,1));
        auto tex = blank->texture();
        glBindTexture(GL_TEXTURE_2D,tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D,0);
        m_blankFbo.swap(blank);
    }
}

void RenderContext::render() {
    qint64 framePeriod = elapsed_timer.nsecsElapsed();
    elapsed_timer.restart();
    {
        QMutexLocker locker(&m_contextLock);

        makeCurrent();
        checkLoadShaders();
        checkCreateNoise();
        checkCreateBlankFbo();

        for(auto n : topoSort()) {
            n->render();
        }
    }
    emit renderingFinished();
    //qint64 renderingPeriod = elapsed_timer.nsecsElapsed();
    m_framePeriodLPF += FPS_ALPHA * (framePeriod - m_framePeriodLPF);
    m_rendering.release();
    emit fpsChanged(fps());
}

qreal RenderContext::fps() const {
    return 1000000000/m_framePeriodLPF;
}

void RenderContext::makeCurrent() {
    context->makeCurrent(surface.data());
}

void RenderContext::flush() {
    context->functions()->glFinish();
}

void RenderContext::addVideoNode(VideoNode* n) {
    // It is less clear to me if taking the context lock
    // is necessary here
    QMutexLocker locker(&m_contextLock);
    m_videoNodes.insert(n);
}

void RenderContext::removeVideoNode(VideoNode* n) {
    // Take the context lock to avoid deleting anything
    // required for the current render
    QMutexLocker locker(&m_contextLock);
    m_videoNodes.remove(n);
}

void RenderContext::addSyncSource(QObject *source) {
    m_syncSources.append(source);
    if(m_syncSources.last() != m_currentSyncSource) {
        if(m_currentSyncSource != NULL) disconnect(m_currentSyncSource, SIGNAL(frameSwapped()), this, SLOT(update()));
        m_currentSyncSource = m_syncSources.last();
        connect(m_currentSyncSource, SIGNAL(frameSwapped()), this, SLOT(update()), Qt::DirectConnection);
    }
}

void RenderContext::removeSyncSource(QObject *source) {
    m_syncSources.removeOne(source);
    if(m_syncSources.isEmpty()) {
        disconnect(m_currentSyncSource, SIGNAL(frameSwapped()), this, SLOT(update()));
        m_currentSyncSource = NULL;
        qDebug() << "Removed last sync source, video output will stop now";
    }
    else if(m_syncSources.last() != m_currentSyncSource) {
        disconnect(m_currentSyncSource, SIGNAL(frameSwapped()), this, SLOT(update()));
        m_currentSyncSource = m_syncSources.last();
        connect(m_currentSyncSource, SIGNAL(frameSwapped()), this, SLOT(update()), Qt::DirectConnection);
    }
}

QList<VideoNode*> RenderContext::topoSort()
{
    // Fuck this

    auto sortedNodes = QList<VideoNode*>{};
    auto fwdEdges = std::map<VideoNode*, QSet<VideoNode*> >{};
    auto revEdges = std::map<VideoNode*, int>{};

    auto startNodes = std::deque<VideoNode*>{};
    auto videoNodes = m_videoNodes;
    for(auto && n: videoNodes) {
        auto deps = n->dependencies();
        revEdges.emplace(n, deps.size());
        if(deps.empty())
            startNodes.push_back(n);
        else for(auto c : deps)
            fwdEdges[c].insert(n);

    }
    while(!startNodes.empty()) {
        auto n = startNodes.back();
        startNodes.pop_back();
        sortedNodes.append(n);
        auto fwd_it = fwdEdges.find(n);
        if(fwd_it != fwdEdges.end()) {
            for(auto c: fwd_it->second) {
                auto &refcnt = revEdges[c];
                if(!--refcnt)
                    startNodes.push_back(c);
            }
            fwdEdges.erase(fwd_it);
        }
    }
    if(!fwdEdges.empty()) {
        qDebug() << "Cycle detected!";
        return {};
    }
    return sortedNodes;
}

int RenderContext::outputCount() const {
    return m_outputCount;
}


int RenderContext::fboIndex(FboRole role) const {
    switch(role) {
        case PreviewFboRole: return 0;
        case OutputFboRole : return 1;
        default: return -1;
    }
}
QSize RenderContext::fboSize(int i) const
{
    if(i == fboIndex(PreviewFboRole))
        return uiSettings->previewSize();
    if(i == fboIndex(OutputFboRole))
        return uiSettings->outputSize();
    return QSize(0, 0);
}
