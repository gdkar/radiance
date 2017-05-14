#pragma once

#include <functional>
#include <utility>
#include <algorithm>
#include <memory>
#include <type_traits>
#include <vector>
#include <deque>
#include <array>

#include <QOpenGLContext>
#include <QOpenGLTexture>
#include <QOpenGLBuffer>
#include <QOpenGLShader>
#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions>
#include <QOpenGLExtraFunctions>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLFramebufferObject>
#include <QImage>
#include <QColor>
#include <QMutex>
#include <QSet>
using SharedFboPointer = QSharedPointer<QOpenGLFramebufferObject>;
using WeakFboPointer   = QWeakPointer  <QOpenGLFramebufferObject>;
using SharedTexPointer = QSharedPointer<QOpenGLTexture>;
using WeakTexPointer   = QWeakPointer<QOpenGLTexture>;
using ShaderProgramPointer = QSharedPointer<QOpenGLShaderProgram>;

class RenderContext;

class VideoNode : public QObject, protected QOpenGLExtraFunctions{
    Q_OBJECT

public:
    VideoNode(RenderContext *context);
   ~VideoNode() override;
    std::vector<SharedFboPointer> m_fbos;
    std::vector<SharedFboPointer> m_displayFbos;
    std::vector<SharedFboPointer> m_renderFbos;
    virtual QSet<VideoNode*> dependencies();
    QVector<QColor> pixels(int i, QVector<QPointF>);
    RenderContext *context();

//    virtual QOpenGLFramebufferObject *outputFbo (int idx) const;
//    virtual QOpenGLFramebufferObject *displayFbo(int idx) const;
///    virtual QOpenGLFramebufferObject *renderFbo (int idx) const;

    virtual SharedFboPointer &outputFbo(int idx) ;
    virtual SharedFboPointer &displayFbo(int idx);
    virtual SharedFboPointer &renderFbo(int idx) ;


    virtual SharedFboPointer outputFbo(int idx) const;
    virtual SharedFboPointer displayFbo(int idx) const;
    virtual SharedFboPointer renderFbo(int idx) const;

    void resizeFbo(QOpenGLFramebufferObject *&fbo, QSize size);
    template<class Pointer = SharedFboPointer>
    void resizeFbo(Pointer &fbo, QSize size)
    {
        if(!fbo || fbo->size() != size) {
//            auto raw = fbo.release();
//            resizeFbo(raw, size);
//            fbo.reset(raw);
            auto rep = Pointer(new QOpenGLFramebufferObject(size, GL_TEXTURE_2D));
            auto tex = rep->texture();
            glBindTexture(GL_TEXTURE_2D,tex);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glBindTexture(GL_TEXTURE_2D,0);
            using std::swap;
            swap(fbo, rep);
        }
    }
public slots:
    void render();
    bool swap(int i);

protected:
    virtual void initialize() = 0;
    virtual void paint() = 0;
    void blitToRenderFbo();
    RenderContext *m_context;

    void beforeDestruction();

signals:
    void initialized();

private:
    std::vector<QMutex> m_textureLocks;
    std::unique_ptr<std::atomic<bool>[]> m_updated;
    bool m_initialized;

    QMutex m_previewImageLock;
    QImage m_previewImage;
    bool m_previewImageValid;
};
