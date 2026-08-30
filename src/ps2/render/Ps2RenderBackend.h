#pragma once

#ifdef PS2_PLATFORM

#include "ps2/render/Ps2RenderState.h"
#include "ps2/render/Ps2GsQueue.h"

// The PS2 uses a signed 16-bit Z buffer. Keep every submitted depth value in
// the positive half so the GS comparison remains monotonic after sign extension.
constexpr int PS2_GS_Z_MAX = 0x7FFF;

// Upper bound of the GS primitive coordinate system, in pixels: XY are 12.4
// fixed point in a 16-bit field, so the last representable position is
// 65535/16 = 4095.9375. gsKit clamps to exactly this in
// __gsKit_float_to_int_xy, which is why no EE path can emit an out-of-range
// vertex; VU1's FTOI4 has no such clamp, so the microprogram applies this
// bound itself (see the MINIw before FTOI4.xy in Ps2Vu1Terrain.vsm).
constexpr float PS2_GS_XY_MAX = 4095.9375f;


struct Ps2TerrainGpuState
{
    unsigned long long tex0;
    unsigned long long texa;
    unsigned long long alpha;
    unsigned long long test;
    unsigned long long zbuf;
    int textureWidth;
    int textureHeight;
    int primContext;
    int primAlphaEnable;
    int primAAEnable;
    int offsetX;
    int offsetY;
    float viewW;
    float viewH;
    float depthQScale;
    float depthQBias;
    bool forceNearZ;
    Ps2RenderState render;
};

// Resolve deferred texture uploads and snapshot the GS state required by the
// direct VU1 terrain path.  Call this before acquiring Path1: texture/state
// uploads are Path3 work and must be allowed to enter the gsKit queue first.
bool ps2_render_prepare_terrain_gpu_state(Ps2TerrainGpuState& out);

// VU1 writes the terrain raster state directly through Path1, bypassing the
// backend cache. Invalidate cached register assumptions before Path3 resumes.
void ps2_render_invalidate_path1_state();

// gsKit_setactive() rewrites FRAME_1/FRAME_2 after a double-buffer flip and
// clears FBMSK in the process. Invalidate the cached framebuffer mask so the
// next draw restores ColorMask on the newly selected backbuffer.
void ps2_render_invalidate_framebuffer_state();

// Path ownership helpers. Terrain VU1 packets use GIF Path1 (XGKICK), while
// gsKit uses Path3. The two paths must be explicitly drained at ownership
// transitions so a later Path3 register packet cannot overtake terrain output.
bool ps2_render_acquire_path1();
void ps2_render_release_path1();
void ps2_render_barrier_path1();



#endif // PS2_PLATFORM
