#include "WaveformUI.h"
#include "main.h"

#include <QQuickFramebufferObject>
#include <QOpenGLFunctions>

class WaveformRenderer : public QQuickFramebufferObject::Renderer, protected QOpenGLFunctions {
    std::unique_ptr<QOpenGLShaderProgram> m_program{};
protected:
    QString m_fragmentShader{};
public:
    WaveformRenderer()
    {
        initializeOpenGLFunctions();
    }
    ~WaveformRenderer() = default;
protected:
    void changeProgram(QString fragmentShader)
    {
        try {
            auto program = RenderContext::defaultVertexHalf();
            if(!program)
                throw std::runtime_error("bad vertex shader.");
            if(!program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShader))
                throw std::runtime_eror("bad fragment shader.");
            if(!program->link())
                throw std::runtime_error("linkage failure.");
            m_fragmentShader.swap(fragmentShader);
            m_program.swap(program);
            m_program->setUniformValue("iWaveform", 0);
        }catch(const std::exception &e) {
            qDebug() << "Error setting shader program" << e.message();
        }
    }
    QOpenGLFramebufferObject *createFramebufferObject(const QSize &size) override
    {
        return new QOpenGLFramebufferObject(size);
    }
    void synchronize(QQuickFramebufferObject *item) override
    {
        auto waveformUI = static_cast<WaveformUI *>(item);
        auto fs = waveformUI->fragmentShader();
        if(fs != m_fragmentShader)
            changeProgram(fs);
    }
    void render() override
    {
        if(m_program) {
            audio->renderWaveform();

            glClearColor(0, 0, 0, 0);
            glClear(GL_COLOR_BUFFER_BIT);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_BLEND);
            m_program->bind();
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_1D, audio->m_waveformTexture->textureId());
            m_program->setUniformValue("iResolution", framebufferObject()->size());
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            m_program->release();
        }
        update();
    }

};
QString WaveformUI::fragmentShader()
{
    QMutexLocker locker(&m_fragmentShaderLock);
    return m_fragmentShader;
}
void WaveformUI::setFragmentShader(QString fragmentShader)
{
    {
        QMutexLocker locker(&m_fragmentShaderLock);
        if(m_fragmentShader == fragmentShader)
            return;
        m_fragmentShader = fragmentShader;
    }
    emit fragmentShaderChanged(fragmentShader);
}
QQuickFramebufferObject::Renderer *WaveformUI::createRenderer() const
{
    return new WaveformRenderer();
}
