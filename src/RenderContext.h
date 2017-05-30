#pragma once

#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QTimer>
#include <QElapsedTimer>
#include <QMutex>
#include <QEvent>
#include <QSet>
#include <QOpenGLShaderProgram>
#include <QOpenGLShader>
#include <QOpenGLTexture>
#include <QSemaphore>
#include "OffscreenContext.h"
#include "VideoNode.h"
#include "semaphore.hpp"

using SharedFboPointer = QSharedPointer<QOpenGLFramebufferObject>;
using WeakFboPointer   = QWeakPointer  <QOpenGLFramebufferObject>;
using SharedTexPointer = QSharedPointer<QOpenGLTexture>;
using WeakTexPointer   = QWeakPointer<QOpenGLTexture>;
using ShaderProgramPointer = QSharedPointer<QOpenGLShaderProgram>;

class RenderContext : public QObject {
    Q_OBJECT

public:
    enum FboRole {
        PreviewFboRole,
        OutputFboRole,
    };
    Q_ENUM(FboRole);
    RenderContext();
   ~RenderContext() override;
    static QString defaultVertexShaderSource();
    static QOpenGLShader *defaultVertexShader();
    static QOpenGLShaderProgram *defaultVertexHalf();
//    QSharedPointer<QOffscreenSurface> surface;
//    QOpenGLContext *context;
    OffscreenContext *context;
    OffscreenContext *compiler_context;
//    QSharedPointer<QOffscreenSurface> compiler_surface;
//    QOpenGLContext *compiler_context;
    QMutex m_compilerLock;
    QTimer *timer;
    QElapsedTimer elapsed_timer;
    QMutex m_contextLock;

    void makeCurrent();
    void flush();

    ShaderProgramPointer m_premultiply;

    QSet<VideoNode*> m_videoNodes; // temp
    int outputCount()const ;
    int fboIndex(FboRole role) const;
    QSize fboSize(int i)const ;
    SharedTexPointer  noiseTexture(int i) const;
    SharedFboPointer  blankFbo() const;

public slots:
    void start();
    void update();
    void addVideoNode(VideoNode* n);
    void removeVideoNode(VideoNode* n);
    void addSyncSource(QObject *source);
    void removeSyncSource(QObject *source);
    qreal fps() const;

private slots:
    void render();

private:
    static constexpr const qreal FPS_ALPHA = 0.03;
    QList<VideoNode*> topoSort();
    void load();
    int m_outputCount;
    QList<QObject *> m_syncSources;
    QObject *m_currentSyncSource;
    radiance::RSemaphore m_rendering;
    QVector<SharedTexPointer> m_noiseTextures;
    void checkLoadShaders();
    void checkCreateNoise();
    void checkCreateBlankFbo();
    SharedFboPointer m_blankFbo;
    qreal m_framePeriodLPF;

signals:
    void renderingFinished();
    void addVideoNodeRequested(VideoNode *n);
    void removeVideoNodeRequested(VideoNode *n);
    void renderRequested();
    void fpsChanged(qreal value);
};
