#pragma once

#include "java/Type.h"

// net.minecraft.src.CanvasCrashReport
// Java: extends Canvas, sets a fixed preferred/minimum size.
// SDL2: no AWT Canvas, so this is a simple size container.
class CanvasCrashReport
{
public:
    CanvasCrashReport(int_t size);

    int_t preferredWidth;
    int_t preferredHeight;
};
