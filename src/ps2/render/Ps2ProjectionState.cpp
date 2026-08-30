#ifdef PS2_PLATFORM

#include "ps2/render/Ps2ProjectionState.h"

#include "ps2/render/Ps2RenderApi.h"

namespace
{
    constexpr int kProjectionStackDepth = 32;
    constexpr unsigned int kProjectionMatrix = ps2RenderValue(Ps2RenderMatrixMode::Projection);

    Ps2ProjectionState s_stack[kProjectionStackDepth];
    int s_top = 0;
    int s_overflow = 0;
}

Ps2ProjectionState g_ps2ProjectionState;

void ps2_projection_set_ortho(float left, float right, float bottom, float top,
                              float nearPlane, float farPlane)
{
    g_ps2ProjectionState.left = left;
    g_ps2ProjectionState.right = right;
    g_ps2ProjectionState.bottom = bottom;
    g_ps2ProjectionState.top = top;
    g_ps2ProjectionState.nearPlane = nearPlane;
    g_ps2ProjectionState.farPlane = farPlane;
    g_ps2ProjectionState.orthographic = true;
}

void ps2_projection_clear_ortho()
{
    g_ps2ProjectionState.orthographic = false;
}

void ps2_projection_push(unsigned int matrixMode)
{
    if (matrixMode != kProjectionMatrix)
        return;

    if (s_overflow > 0 || s_top >= kProjectionStackDepth - 1)
    {
        ++s_overflow;
        return;
    }

    s_stack[s_top++] = g_ps2ProjectionState;
}

void ps2_projection_pop(unsigned int matrixMode)
{
    if (matrixMode != kProjectionMatrix)
        return;

    if (s_overflow > 0)
    {
        --s_overflow;
        return;
    }

    if (s_top > 0)
        g_ps2ProjectionState = s_stack[--s_top];
}

#endif // PS2_PLATFORM
