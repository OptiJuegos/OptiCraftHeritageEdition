#pragma once

#if defined(PS2_PLATFORM)

#include <gsKit.h>
#include <gsTexture.h>

#include "ps2/render/Ps2NativeDraw.h"
#include "ps2/render/Ps2RenderState.h"

struct Ps2Draw2DState {
    GSGLOBAL* gsGlobal;
    GSTEXTURE* texture;
    const float* modelView;
    Ps2RenderState render;

    float left;
    float right;
    float bottom;
    float top;
    float nearPlane;
    float farPlane;
    float viewW;
    float viewH;
    int depthMax;
    bool depthTest;

    void (*queueGuard)(int vertexCount);
    void (*applyClampSel)(int mode, int ufix, int vfix);

#ifdef PS2_RENDER_STATS
    // Quads taken by the GS sprite path vs quads that had to stay on the
    // triangle path. The ratio is the whole point of the sprite emitter: if the
    // GUI is not mostly sprites, it is not paying for itself.
    long* debugSprites;
    long* debugSpriteFallbacks;
    long* debugDepthTested;
    long* debugDepthPinned;
    int* debugDepthLow;
    int* debugDepthHigh;
    float* debugEyeZLow;
    float* debugEyeZHigh;
    float* debugNear;
    float* debugFar;
#endif
};

// Emits an already-decoded orthographic mesh directly to gsKit. No OpenGL
// client arrays or glDrawArrays state are programmed on this path.
bool ps2_draw_2d_mesh(const Ps2NativeMeshView& mesh, const Ps2Draw2DState& state);

#endif // PS2_PLATFORM
