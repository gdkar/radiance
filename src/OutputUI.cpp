#include "OutputUI.h"
#include "main.h"

#include <QtCore/QMutex>
#include <QOpenGLWindow>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QScreen>
#include <QGuiApplication>

class OutputWindow : public QOpenGLWindow, protected QOpenGLFunctions {
    Q_OBJECT

protected slots:
    void putOnScreen() {
        setGeometry(screen()->geometry());
    }

public:
    OutputWindow(OutputUI *outputUI)
        : QOpenGLWindow(QOpenGLContext::globalShareContext())
        , m_outputUI(outputUI)
        , m_program(0) {
        setFlags(Qt::Dialog);
        setWindowState(Qt::WindowFullScreen);
        putOnScreen();
        connect(this, &QWindow::screenChanged, this, &OutputWindow::putOnScreen);
    }

    ~OutputWindow() {
        makeCurrent();
        teardownGL();
    }

    void initializeGL() {
        // Initialize OpenGL Backend
        initializeOpenGLFunctions();

        // Set global information
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        auto program = RenderContext::defaultVertexHalf();
        program->addShaderFromSourceCode(QOpenGLShader::Fragment,
                                        "#version 130\n"
                                        "uniform vec2 iResolution;\n"
                                        "uniform sampler2D iFrame;\n"
                                        "in vec2 uv;\n"
                                        "out vec4 fragColor;\n"
                                        "void main(void) {\n"
                                        "    fragColor = texture2D(iFrame, uv);\n"
                                        "    fragColor.a = 1.;\n"
                                        "}");
        program->link();

        m_program = program;
    }

    void resizeGL(int width, int height) {
    }
    void paintGL() {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        QMutexLocker locker(&m_outputUI->m_sourceLock);

        if(m_outputUI->m_source ) {
            auto m_videoNode = m_outputUI->m_source->m_videoNode;
            auto out_index = m_videoNode->context()->fboIndex(RenderContext::OutputFboRole);
            m_videoNode->swap(out_index);
            if(m_videoNode->m_displayFbos[out_index]) {

                glClear(GL_COLOR_BUFFER_BIT);
                glDisable(GL_DEPTH_TEST);
                glDisable(GL_BLEND);

                m_program->bind();
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, m_videoNode->m_displayFbos[out_index]->texture());
                m_program->setUniformValue("iResolution", size());
                m_program->setUniformValue("iFrame", 0);
                glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
                m_program->release();
            }
        }
        glFinish();
        update();
    }

    void teardownGL() {
        delete m_program;
    }

    OutputUI *m_outputUI;
    QOpenGLShaderProgram *m_program;
};

// OutputUI

OutputUI::OutputUI()
    : m_source(0)
    , m_screen(0) {
    m_outputWindow = new OutputWindow(this);
    m_screen = m_outputWindow->screen();
    connect(m_outputWindow, &QWindow::screenChanged, this, &OutputUI::onScreenChanged);
    connect(m_outputWindow, &QWindow::visibleChanged, this, &OutputUI::visibleChanged);
    connect(m_outputWindow, &QWindow::visibleChanged, this, &OutputUI::onVisibleChanged);
    if(auto app = qobject_cast<QGuiApplication*>(QCoreApplication::instance())){
        connect(app, &QGuiApplication::screenRemoved, this, &OutputUI::availableScreensChanged);
        connect(app, &QGuiApplication::screenAdded,   this, &OutputUI::availableScreensChanged);
    }
}

OutputUI::~OutputUI() {
    delete m_outputWindow;
    m_outputWindow = 0;
}

QString OutputUI::screen() {
    return m_outputWindow->screen()->name();
}

void OutputUI::onScreenChanged(QScreen *screen) {
    if(screen != m_screen) {
        m_outputWindow->setScreen(m_screen);
    }
}

void OutputUI::setScreen(QString screenName) {
    foreach(QScreen *screen, QGuiApplication::screens()) {
        if(screen->name() == screenName) {
            m_screen = screen;
            m_outputWindow->setScreen(screen);
            emit screenChanged(screen->name());
            break;
        }
    }
}

void OutputUI::setVisible(bool visible) {
    m_outputWindow->setVisible(visible);
}

bool OutputUI::visible() {
    return m_outputWindow->isVisible();
}

VideoNodeUI *OutputUI::source() {
    QMutexLocker locker(&m_sourceLock);
    return m_source;
}

void OutputUI::setSource(VideoNodeUI *value) {
    {
        QMutexLocker locker(&m_sourceLock);
        if(m_source == value) return;
        m_source = value;
    }
    emit sourceChanged(value);
}

QStringList OutputUI::availableScreens() {
    QStringList result;
    foreach(QScreen *screen, QGuiApplication::screens()) {
        result.append(screen->name());
    }
    return result;
}

void OutputUI::onVisibleChanged(bool visible) {
    if(visible) {
        renderContext->addSyncSource(m_outputWindow);
    } else {
        renderContext->removeSyncSource(m_outputWindow);
    }
}

#include "OutputUI.moc"
