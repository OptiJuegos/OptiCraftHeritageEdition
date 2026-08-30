#pragma once

#include "CanvasIsomPreview.h"

// net.minecraft.isom.IsomPreviewApplet
class IsomPreviewApplet
{
public:
    IsomPreviewApplet() = default;

    void start();
    void stop();

    CanvasIsomPreview *getPreview();

private:
    CanvasIsomPreview isomPreview;
};
