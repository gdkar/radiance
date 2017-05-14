#include "VideoNodeUI.h"
#include "main.h"
#include <memory>
#include <QtCore/QMutex>
#include <QQuickFramebufferObject>

class VideoNodeRenderer : public QQuickFramebufferObject::Renderer, protected QOpenGLFunctions {
public:
    VideoNodeRenderer(VideoNode *videoNode)
        : m_videoNode(videoNode){

        initializeOpenGLFunctions();

        auto program = ShaderProgramPointer(RenderContext::defaultVertexHalf());
        program->addShaderFromSourceCode(QOpenGLShader::Fragment,
                                        "#version 130\n"
                                        "in vec2 uv;\n"
                                        "uniform vec2 iResolution;\n"
                                        "uniform sampler2D iFrame;\n"
                                        "out vec4 fragColor;\n"
                                        "void main(void) {\n"
                                        "    fragColor = texture2D(iFrame, vec2(uv.x,1.-uv.y));\n"
                                        "}\n");
        program->link();
        m_program.swap(program);
    }
    ~VideoNodeRenderer()
    {
        m_program.reset();
    }
protected:
    QOpenGLFramebufferObject *createFramebufferObject(const QSize &size) override {
        return new QOpenGLFramebufferObject(size);
    }

    void render() override {

        auto pre_index = m_videoNode->context()->fboIndex(RenderContext::PreviewFboRole);
        m_videoNode->swap(pre_index);
        if(m_videoNode->m_displayFbos[pre_index]) {

            glClearColor(0, 0, 0, 0);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_BLEND);

            m_program->bind();
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, m_videoNode->m_displayFbos[pre_index]->texture());
            m_program->setUniformValue("iResolution", framebufferObject()->size());
            m_program->setUniformValue("iFrame", 0);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            m_program->release();
        }
        update();
    }

    VideoNode *m_videoNode{};
    ShaderProgramPointer m_program{};
};

// VideoNodeUI

VideoNodeUI::VideoNodeUI()
    : m_videoNode(0) {
    //setMirrorVertically(true);
}

VideoNodeUI::~VideoNodeUI() {
    delete m_videoNode;
    m_videoNode = 0;
}

QQuickFramebufferObject::Renderer *VideoNodeUI::createRenderer() const {
    return new VideoNodeRenderer(m_videoNode);
}

void VideoNodeUI::setFps(qreal value) {
    m_fps = value;
    emit fpsChanged(value);
}

qreal VideoNodeUI::fps() {
    return m_fps;
}
