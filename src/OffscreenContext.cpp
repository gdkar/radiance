#include "OffscreenContext.h"

OffscreenContext::OffscreenContext(QScreen *screen , QObject *parent )
: QOpenGLContext(parent)
, m_surface(screen)
{
    m_surface.setParent(this);
    if(auto scontext = globalShareContext())
        setShareContext(scontext);
}
void OffscreenContext::setFormat(const QSurfaceFormat &fmt)
{
    super::setFormat(fmt);
    m_surface.setFormat(fmt);
}
bool OffscreenContext::create()
{
    if(!super::create()) {
        return false;
    }
    m_surface.setFormat(format());
    m_surface.create();
    return m_surface.isValid();
}
bool OffscreenContext::makeCurrent()
{
    return super::makeCurrent(&m_surface);
}
