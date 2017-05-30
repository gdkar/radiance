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
#include "VideoNode.h"
#include "semaphore.hpp"

class OffscreenContext : public QOpenGLContext {
    Q_OBJECT

protected:
    QOffscreenSurface m_surface{};
public:
    using super = QOpenGLContext;
    explicit OffscreenContext(QScreen *screen = nullptr, QObject *parent = nullptr);
    void setFormat(const QSurfaceFormat &fmt);
    bool makeCurrent();
    bool create();
};
