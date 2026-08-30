#ifdef PS2_PLATFORM

#include "ps2/render/Ps2Viewport.h"

#include <gsKit.h>
#include <gsMisc.h>

namespace
{
    float s_width = 0.0f;
    float s_height = 0.0f;
    int s_x = 0;
    int s_y = 0;
    int s_baseOffsetX = -1;
    int s_baseOffsetY = -1;
}

extern GSGLOBAL* gsGlobal;

float ps2_viewport_width()
{
    if (s_width > 0.0f)
        return s_width;
    return gsGlobal != nullptr ? static_cast<float>(gsGlobal->Width) : 0.0f;
}

float ps2_viewport_height()
{
    if (s_height > 0.0f)
        return s_height;
    return gsGlobal != nullptr ? static_cast<float>(gsGlobal->Height) : 0.0f;
}

void ps2_viewport_set(int x, int y, int width, int height)
{
    if (gsGlobal == nullptr)
        return;

    s_x = x;
    s_y = y;
    if (s_baseOffsetX < 0)
    {
        s_baseOffsetX = gsGlobal->OffsetX;
        s_baseOffsetY = gsGlobal->OffsetY;
    }

    if (width <= 0 || height <= 0 ||
        (x == 0 && y == 0 && width >= static_cast<int>(gsGlobal->Width) &&
         height >= static_cast<int>(gsGlobal->Height)))
    {
        s_x = 0;
        s_y = 0;
        s_width = 0.0f;
        s_height = 0.0f;
        gsGlobal->OffsetX = s_baseOffsetX;
        gsGlobal->OffsetY = s_baseOffsetY;
        gsKit_set_scissor(gsGlobal,
            GS_SETREG_SCISSOR(0, gsGlobal->Width - 1, 0, gsGlobal->Height - 1));
        return;
    }

    s_width = static_cast<float>(width);
    s_height = static_cast<float>(height);
    gsGlobal->OffsetX = s_baseOffsetX + (x << 4);
    gsGlobal->OffsetY = s_baseOffsetY + (y << 4);
    gsKit_set_scissor(gsGlobal,
        GS_SETREG_SCISSOR(x, x + width - 1, y, y + height - 1));
}

void ps2_viewport_get(int* values)
{
    if (values == nullptr)
        return;

    values[0] = s_x;
    values[1] = s_y;
    values[2] = static_cast<int>(ps2_viewport_width());
    values[3] = static_cast<int>(ps2_viewport_height());
}

#endif // PS2_PLATFORM
