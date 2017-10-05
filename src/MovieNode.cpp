#include "MovieNode.h"
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include "main.h"

MovieNode::MovieNode()
    : m_ready(false)
    , m_videoPath()
    , m_renderFbos()
    , m_openGLWorkerContext(nullptr)
    , m_videoSize(0, 0) {

    m_openGLWorkerContext = new OpenGLWorkerContext();
    m_openGLWorkerContext->setParent(this);

    m_openGLWorker = new MovieNodeOpenGLWorker(this);

    connect(m_openGLWorker, &MovieNodeOpenGLWorker::initialized, this, &MovieNode::onInitialized);
    connect(this, &MovieNode::videoPathChanged, m_openGLWorker, &MovieNodeOpenGLWorker::onVideoChanged);
    connect(m_openGLWorker, &MovieNodeOpenGLWorker::videoSizeChanged, this, &MovieNode::setVideoSize);
    connect(this, &QObject::destroyed, m_openGLWorker, &QObject::deleteLater, Qt::DirectConnection);

    bool result = QMetaObject::invokeMethod(m_openGLWorker, "initialize");
    Q_ASSERT(result);
}

MovieNode::MovieNode(const MovieNode &other)
    : VideoNode(other)
    , m_videoPath(other.m_videoPath)
    , m_openGLWorker(other.m_openGLWorker)
    , m_ready(other.m_ready)
    , m_blitShader(other.m_blitShader)
    , m_renderFbos()
    , m_videoSize(other.m_videoSize) {

    auto k = other.m_renderFbos.keys();
    for (int i=0; i<k.count(); i++) {
        auto otherRenderFbo = other.m_renderFbos.value(k.at(i));
        m_renderFbos.insert(k.at(i), QSharedPointer<QOpenGLFramebufferObject>(otherRenderFbo));
    }
}

MovieNode::~MovieNode() {
}

void MovieNode::onInitialized() {
    m_ready = true;
}

void MovieNode::chainsEdited(QList<QSharedPointer<Chain>> added, QList<QSharedPointer<Chain>> removed) {
    QMutexLocker locker(&m_stateLock);
    for (int i=0; i<added.count(); i++) {
        m_renderFbos.insert(added.at(i), QSharedPointer<QOpenGLFramebufferObject>());
    }
    for (int i=0; i<removed.count(); i++) {
        m_renderFbos.remove(removed.at(i));
    }
}

QString MovieNode::videoPath() {
    Q_ASSERT(QThread::currentThread() == thread());
    return m_videoPath;
}

void MovieNode::setVideoPath(QString videoPath) {
    Q_ASSERT(QThread::currentThread() == thread());
    if(videoPath != m_videoPath) {
        {
            QMutexLocker locker(&m_stateLock);
            m_videoPath = videoPath;
        }

        emit videoPathChanged(videoPath);
    }
}

QSize MovieNode::videoSize() {
    Q_ASSERT(QThread::currentThread() == thread());
    return m_videoSize;
}

void MovieNode::setVideoSize(QSize size) {
    Q_ASSERT(QThread::currentThread() == thread());
    if(size != m_videoSize) {
        {
            QMutexLocker locker(&m_stateLock);
            m_videoSize = size;
        }

        emit videoSizeChanged(size);
    }
}

// See comments in MovieNode.h about these 3 functions
QSharedPointer<VideoNode> MovieNode::createCopyForRendering() {
    return QSharedPointer<VideoNode>(new MovieNode(*this));
}

static void *get_proc_address(void *ctx, const char *name) {
    Q_UNUSED(ctx);
    QOpenGLContext *glctx = QOpenGLContext::currentContext();
    if (!glctx)
        return NULL;
    return (void *)glctx->getProcAddress(QByteArray(name));
}

GLuint MovieNode::paint(QSharedPointer<Chain> chain, QVector<GLuint> inputTextures) {
    // Hitting this assert means
    // that you failed to make a copy
    // of the VideoNode
    // before rendering in a different thread
    Q_ASSERT(QThread::currentThread() == thread());

    GLuint outTexture = 0;

    if (!m_ready) {
        qDebug() << this << "is not ready";
        return outTexture;
    }
    if (!m_renderFbos.contains(chain)) {
        qDebug() << this << "does not have chain" << chain;
        return outTexture;
    }
    auto renderFbo = m_renderFbos.value(chain);

    // FBO creation must happen here, and not in initialize,
    // because FBOs are not shared among contexts.
    // Textures are, however, so in the future maybe we can move
    // texture creation to initialize()
    // and leave lightweight FBO creation here
    if(renderFbo.isNull()) {
        m_renderFbos[chain] = QSharedPointer<QOpenGLFramebufferObject>(new QOpenGLFramebufferObject(chain->size()));
        renderFbo = m_renderFbos.value(chain);
    }

    glClearColor(0, 0, 0, 0);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    renderFbo->bind();
    m_blitShader->bind();
    glActiveTexture(GL_TEXTURE0);
    {
        int i = m_openGLWorker->lastIndex();
        QMutexLocker locker(m_openGLWorker->m_fboLocks.at(i));
        if (m_openGLWorker->m_fbos.at(i) == nullptr) return outTexture;
        glBindTexture(GL_TEXTURE_2D, m_openGLWorker->m_fbos.at(i)->texture());
        m_blitShader->setUniformValue("iVideoFrame", 0);
        m_blitShader->setUniformValue("iResolution", GLfloat(chain->size().width()), GLfloat(chain->size().height()));
        m_blitShader->setUniformValue("iVideoResolution", GLfloat(m_videoSize.width()), GLfloat(m_videoSize.height()));
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
    m_blitShader->release();
    renderFbo->release();
    outTexture = renderFbo->texture();

    return outTexture;
}

void MovieNode::copyBackRenderState(QSharedPointer<Chain> chain, QSharedPointer<VideoNode> copy) {
    QSharedPointer<MovieNode> c = qSharedPointerCast<MovieNode>(copy);
    QMutexLocker locker(&m_stateLock);
    if (m_renderFbos.contains(chain)) {
        m_renderFbos[chain] = c->m_renderFbos.value(chain);
    } else {
    }
}

// MovieNodeOpenGLWorker methods

MovieNodeOpenGLWorker::MovieNodeOpenGLWorker(MovieNode *p)
    : OpenGLWorker(p->m_openGLWorkerContext)
    , m_p(p)
    , m_fbos(BUFFER_COUNT)
    , m_fboLocks(BUFFER_COUNT)
    , m_fboIndex(0)
    , m_mpv_gl(nullptr)
    , m_size(0, 0) {

    for (int i=0; i<BUFFER_COUNT; i++) {
        m_fboLocks[i] = new QMutex();
    }

    connect(this, &MovieNodeOpenGLWorker::message, m_p, &MovieNode::message);
    connect(this, &MovieNodeOpenGLWorker::warning, m_p, &MovieNode::warning);
    connect(this, &MovieNodeOpenGLWorker::fatal,   m_p, &MovieNode::fatal);
}

static void requestUpdate(void *ctx) {
    QMetaObject::invokeMethod((MovieNodeOpenGLWorker*)ctx, "drawFrame", Qt::QueuedConnection);
}

static void requestWakeup(void *ctx) {
    QMetaObject::invokeMethod((MovieNodeOpenGLWorker*)ctx, "onEvent", Qt::QueuedConnection);
}

void MovieNodeOpenGLWorker::initialize() {
    setlocale(LC_NUMERIC, "C");
    m_mpv = mpv::qt::Handle::FromRawHandle(mpv_create());
    if (!m_mpv)
        throw std::runtime_error("could not create mpv context");

    mpv_set_option_string(m_mpv, "terminal", "yes");
    mpv_set_option_string(m_mpv, "msg-level", "all=v");
    mpv_set_option_string(m_mpv, "ytdl", "yes");
    if (mpv_initialize(m_mpv) < 0)
        throw std::runtime_error("could not initialize mpv context");

    //mpv_set_property_string(m_mpv, "video-sync", "display-resample");
    //mpv_set_property_string(m_mpv, "display-fps", "60");
    //mpv_set_property_string(m_mpv, "hwdec", "auto");
    mpv_set_property_string(m_mpv, "loop", "inf");

    // Make use of the MPV_SUB_API_OPENGL_CB API.
    mpv::qt::set_option_variant(m_mpv, "vo", "opengl-cb");

    m_mpv_gl = (mpv_opengl_cb_context *)mpv_get_sub_api(m_mpv, MPV_SUB_API_OPENGL_CB);
    if (!m_mpv_gl)
        throw std::runtime_error("OpenGL not compiled in");

    mpv_observe_property(m_mpv, 0, "duration", MPV_FORMAT_DOUBLE);
    mpv_observe_property(m_mpv, 0, "time-pos", MPV_FORMAT_DOUBLE);
    mpv_observe_property(m_mpv, 0, "video-params/w", MPV_FORMAT_INT64);
    mpv_observe_property(m_mpv, 0, "video-params/h", MPV_FORMAT_INT64);

    int r = mpv_opengl_cb_init_gl(m_mpv_gl, NULL, get_proc_address, NULL);
    if (r < 0)
        throw std::runtime_error("could not initialize OpenGL");

    mpv_opengl_cb_set_update_callback(m_mpv_gl, requestUpdate, (void *)this);

    mpv_set_wakeup_callback(m_mpv, requestWakeup, this);

    if (!loadBlitShader()) return;

    emit initialized();

    onVideoChanged();
}

void MovieNodeOpenGLWorker::onEvent() {
    while (m_mpv) {
        mpv_event *event = mpv_wait_event(m_mpv, 0);
        if (event->event_id == MPV_EVENT_NONE) {
            break;
        }
        handleEvent(event);
    }
}

void MovieNodeOpenGLWorker::handleEvent(mpv_event *event) {
    switch (event->event_id) {
    case MPV_EVENT_PROPERTY_CHANGE: {
        mpv_event_property *prop = (mpv_event_property *)event->data;
        if (strcmp(prop->name, "time-pos") == 0) {
            if (prop->format == MPV_FORMAT_DOUBLE) {
                double time = *(double *)prop->data;
                Q_EMIT positionChanged(time);
            }
        } else if (strcmp(prop->name, "duration") == 0) {
            if (prop->format == MPV_FORMAT_DOUBLE) {
                double time = *(double *)prop->data;
                Q_EMIT durationChanged(time);
            }
        } else if (strcmp(prop->name, "video-params/w") == 0) {
            if (prop->format == MPV_FORMAT_INT64) {
                qint64 d = *(qint64 *)prop->data;
                m_size.setWidth(d);
                emit videoSizeChanged(m_size);
            }
        } else if (strcmp(prop->name, "video-params/h") == 0) {
            if (prop->format == MPV_FORMAT_INT64) {
                qint64 d = *(qint64 *)prop->data;
                m_size.setHeight(d);
                emit videoSizeChanged(m_size);
            }
        }
        break;
    }
    default: ;
        // Ignore uninteresting or unknown events.
    }
}

void MovieNodeOpenGLWorker::drawFrame() {
    {
        QMutexLocker locker(m_fboLocks[m_fboIndex]);
        if (m_fbos.at(m_fboIndex) == nullptr || m_fbos.at(m_fboIndex)->size() != m_size) {
            delete m_fbos.at(m_fboIndex);
            m_fbos[m_fboIndex] = new QOpenGLFramebufferObject(m_size);
        }
        auto fbo = m_fbos.at(m_fboIndex);

        mpv_opengl_cb_draw(m_mpv_gl, fbo->handle(), fbo->width(), -fbo->height());
        glFlush();
        m_fboIndex = (m_fboIndex + 1) % BUFFER_COUNT;
    }
}

int MovieNodeOpenGLWorker::lastIndex() {
    return (m_fboIndex + BUFFER_COUNT - 1) % BUFFER_COUNT;
}

void MovieNodeOpenGLWorker::onVideoChanged() {
    qDebug() << "LOAD" << m_p->m_videoPath;
    if (!m_mpv_gl) return; // Wait for initialization
    QString filename;
    {
        QMutexLocker locker(&m_p->m_stateLock);
        if (m_p->m_videoPath.isEmpty()) return;
        //filename = QString("../resources/videos/%1").arg(m_p->m_videoPath);
        filename = QString("%1").arg(m_p->m_videoPath);
    }

    QFileInfo check_file(filename);
    if(!(check_file.exists() && check_file.isFile())) {
        qWarning() << "Could not find" << filename;
        emit warning(QString("Could not find %1").arg(filename));
    }

    command(QStringList() << "loadfile" << filename);

    qDebug() << "Successfully loaded video" << filename;
}

void MovieNodeOpenGLWorker::command(const QVariant &params) {
    mpv::qt::command_variant(m_mpv, params);
}

MovieNodeOpenGLWorker::~MovieNodeOpenGLWorker() {
    if (m_mpv_gl)
        mpv_opengl_cb_set_update_callback(m_mpv_gl, NULL, NULL);
    // Until this call is done, we need to make sure the player remains
    // alive. This is done implicitly with the mpv::qt::Handle instance
    // in this class.
    mpv_opengl_cb_uninit_gl(m_mpv_gl);
    qDeleteAll(m_fbos.begin(), m_fbos.end());
    qDeleteAll(m_fboLocks.begin(), m_fboLocks.end());
}

bool MovieNodeOpenGLWorker::loadBlitShader() {
    auto vertexString = QString{
        "#version 130\n"
        "#extension GL_ARB_shading_language_420pack : enable\n"
        "const vec2 varray[4] = { vec2( 1., 1.),vec2(1., -1.),vec2(-1., 1.),vec2(-1., -1.)};\n"
        "out vec2 coords;\n"
        "void main() {\n"
        "    vec2 vertex = varray[gl_VertexID];\n"
        "    gl_Position = vec4(vertex,0.,1.);\n"
        "    coords = vertex;\n"
        "}\n"};
    auto fragmentString = QString{
        "#version 130\n"
        "#extension GL_ARB_shading_language_420pack : enable\n"
        "uniform sampler2D iVideoFrame;\n"
        "uniform vec2 iResolution;\n"
        "uniform vec2 iVideoResolution;\n"
        "vec2 uv = gl_FragCoord.xy / iResolution;\n"
        "void main() {\n"
        "    vec2 factorFit = iVideoResolution.yx * iResolution.xy / iVideoResolution.xy / iResolution.yx;\n"
        "    vec2 factor = max(factorFit, 1.);\n"
        "    vec2 texUV = (uv - 0.5) * factor + 0.5;\n"
        "    vec2 clamp = (step(0., texUV) - step(1., texUV));\n"
        "    gl_FragColor = texture2D(iVideoFrame, texUV) * clamp.x * clamp.y;\n"
        "}\n"};

    m_p->m_blitShader = QSharedPointer<QOpenGLShaderProgram>(new QOpenGLShaderProgram());
    if(!m_p->m_blitShader->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexString)) {
        emit fatal("Could not compile vertex shader");
        return false;
    }
    if(!m_p->m_blitShader->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentString)) {
        emit fatal("Could not compile fragment shader");
        return false;
    }
    if(!m_p->m_blitShader->link()) {
        emit fatal("Could not link shader program");
        return false;
    }
    return true;
}
