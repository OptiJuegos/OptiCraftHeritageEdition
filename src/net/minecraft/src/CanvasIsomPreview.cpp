#include "CanvasIsomPreview.h"

#include "ThreadRunIsoClient.h"

CanvasIsomPreview::CanvasIsomPreview()
    : running(true)
{
}

CanvasIsomPreview::~CanvasIsomPreview()
{
    stop();
    uiThread.reset();
}

void CanvasIsomPreview::start()
{
    if (uiThread != nullptr)
        return;

    running = true;
    uiThread = std::make_unique<ThreadRunIsoClient>(this);
    uiThread->start();
}

void CanvasIsomPreview::stop()
{
    running = false;
}

void CanvasIsomPreview::showNextBuffer()
{
    // The native port has no AWT BufferStrategy. Rendering the isometric
    // preview remains owned by the platform UI; this method only drives the
    // Java-equivalent lifecycle loop.
}
