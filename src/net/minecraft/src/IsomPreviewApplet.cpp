#include "IsomPreviewApplet.h"

void IsomPreviewApplet::start()
{
    isomPreview.start();
}

void IsomPreviewApplet::stop()
{
    isomPreview.stop();
}

CanvasIsomPreview *IsomPreviewApplet::getPreview()
{
    return &isomPreview;
}
