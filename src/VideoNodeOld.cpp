#include "VideoNodeOld.h"
#include "RenderContext.h"
#include <QDebug>
#include <QThread>

VideoNodeOld::VideoNodeOld(RenderContext *context)
    : m_context(context),
    m_fbos(context->outputCount()),
    m_displayFbos(context->outputCount()),
    m_renderFbos(context->outputCount()),
    m_updated(std::make_unique<std::atomic<bool>[] >(context->outputCount())),
    m_textureLocks(context->outputCount()),
    m_initialized(false)
{
    moveToThread(context->thread());
//    for(int i=0; i<m_context->outputCount(); i++)
//        m_textureLocks[i] = new QMutex();
    emit m_context->addVideoNodeOldRequested(this);
}
/*QOpenGLFramebufferObject *VideoNodeOld::outputFbo(int idx) const
{
    return m_fbos.at(idx).get();
}
QOpenGLFramebufferObject *VideoNodeOld::displayFbo(int idx) const
{
    return m_displayFbos.at(idx).get();
}
QOpenGLFramebufferObject *VideoNodeOld::renderFbo(int idx) const
{
    return m_renderFbos.at(idx).get();
}*/
std::shared_ptr<QOpenGLFramebufferObject> &VideoNodeOld::outputFbo(int idx)
{
    return m_fbos[idx];
}
std::shared_ptr<QOpenGLFramebufferObject> &VideoNodeOld::displayFbo(int idx)
{
    return m_displayFbos[idx];
}
std::shared_ptr<QOpenGLFramebufferObject> &VideoNodeOld::renderFbo(int idx)
{
    return m_renderFbos[idx];
}
std::shared_ptr<QOpenGLFramebufferObject> VideoNodeOld::outputFbo(int idx) const
{
    return m_fbos[idx];
}
std::shared_ptr<QOpenGLFramebufferObject> VideoNodeOld::displayFbo(int idx) const
{
    return m_displayFbos[idx];
}
std::shared_ptr<QOpenGLFramebufferObject> VideoNodeOld::renderFbo(int idx) const
{
    return m_renderFbos[idx];
}
void VideoNodeOld::render() {
    if(!m_initialized) {
        initializeOpenGLFunctions(); // Placement of this function is black magic to me
        initialize();
        std::fill_n(&m_updated[0],m_context->outputCount(),false);
        m_initialized = true;
        emit initialized();
    }
    paint();
}

// This function is called from paint()
// to draw onto the back-buffer.
void VideoNodeOld::blitToRenderFbo() {
    m_context->flush(); // Flush before taking the lock to speed things up a bit

    glClearColor(0, 0, 0, 0);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    m_context->m_premultiply->bind();
        m_context->m_premultiply->setUniformValue("iFrame", 0);
    for(int i=0; i<m_context->outputCount(); i++) {
        QMutexLocker locker(&m_textureLocks[i]);
        auto size = outputFbo(i)->size();
        resizeFbo(renderFbo(i), size);

        glViewport(0, 0, size.width(), size.height());

        renderFbo(i)->bind();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, outputFbo(i)->texture());
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        m_context->flush();
        m_updated[i] = true;
    }
    m_context->m_premultiply->release();
    QOpenGLFramebufferObject::bindDefault();

    //QOpenGLFramebufferObject::blitFramebuffer(m_renderPreviewFbo, m_previewFbo);

    //QMutexLocker imageLocker(&m_previewImageLock);
    //m_previewImageValid = false;
}

// This function gets the pixel values at the given points
QVector<QColor> VideoNodeOld::pixels(int i, QVector<QPointF> points) {
    QMutexLocker imageLocker(&m_previewImageLock);
    if (!m_previewImageValid) {
        QMutexLocker locker(&m_textureLocks.at(i));
        m_previewImage = m_fbos.at(i)->toImage();
        m_previewImageValid = true;
    }
    QVector<QColor> output;
    for (auto point : points) {
        auto scaled_point = point;
        scaled_point.rx() *= m_previewImage.width();
        scaled_point.ry() *= m_previewImage.height();
        output.append(m_previewImage.pixel(scaled_point.toPoint()));
    }
    return output;
}

// This function is called from the rendering thread
// to get the latest frame in m_displayFbo[i].
// It returns true if there is a new frame
bool VideoNodeOld::swap(int i) {
    QMutexLocker locker(&m_textureLocks[i]);
    if(m_updated[i].exchange(false)) {
        m_context->flush();
        std::swap(displayFbo(i),renderFbo(i));
        //resizeFbo(&m_displayFbos[i], m_renderFbos.at(i)->size());
        //QOpenGLFramebufferObject::blitFramebuffer(m_displayFbos.at(i), m_renderFbos.at(i));
        return true;
    }
    return false;
}

// Before deleting any FBOs and whatnot, we need to
// 1. Make sure we aren't currently rendering
// 2. Remove ourselves from the context graph
void VideoNodeOld::beforeDestruction() {
    m_context->removeVideoNodeOld(this);
}

VideoNodeOld::~VideoNodeOld() {
    // Stop event processing, move the thread to GUI and make sure it is deleted.
    //moveToThread(QGuiApplication::instance()->thread());
}

void VideoNodeOld::resizeFbo(QOpenGLFramebufferObject *&fbo, QSize size) {

    if(!fbo || fbo->size() != size) {
        auto rep = std::make_unique<QOpenGLFramebufferObject>(size);
        auto tex = rep->texture();
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);
        delete fbo;
        fbo = rep.release();
    }
}

QSet<VideoNodeOld*> VideoNodeOld::dependencies() {
    return QSet<VideoNodeOld*>();
}

RenderContext *VideoNodeOld::context() {
    return m_context;
}
