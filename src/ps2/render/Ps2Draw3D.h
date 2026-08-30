#pragma once

#ifdef PS2_PLATFORM

#include <gsKit.h>
#include <gsTexture.h>

#include "ps2/render/Ps2ClipGuard.h" // Ps2DepthMap
#include "ps2/render/Ps2NativeDraw.h"
#include "ps2/render/Ps2RenderState.h"

struct Ps2Draw3DState {
	bool packedTerrain;
    const Ps2NativeClampRun* clampRuns;
    int clampRunCount;
    bool tileAtlas;
    GSGLOBAL* gsGlobal;
    GSTEXTURE* texture;
    const float* mvp;

    const void* vertices;
    int vertexStride;
    int vertexSize;

    const void* texCoords;
    int texCoordStride;
    bool texCoordEnabled;

    const void* colors;
    int colorStride;
    int colorSize;
    bool colorEnabled;
    bool colorFloat;

    // Raster/shading state is captured at the compatibility boundary. Draw3D
    // never reads gles_ps2.cpp globals directly.
    Ps2RenderState render;

    int first;
    int count;

    // Vertices arrive as GL_QUADS (4 per face) rather than GL_TRIANGLES. Terrain
    // and entity meshes are quads at the source, so this skips both the 4->6
    // expansion in the tessellator and the pattern re-detection the triangle
    // path needs to recover the original corners.
    bool quads;

    bool ortho;

    // Geometry is known to be fully inside the view frustum (chunk AABB test),
    // so per-triangle clip outcodes can be skipped entirely.
    bool fullyInside;

    // Near/far of the active perspective projection, packaged for ps2_gs_depth
    // (Ps2ClipGuard.h). Must be the same mapping the generic path uses -- both
    // write into one shared PSMZ_16S buffer.
    Ps2DepthMap depth;

    // Pin every projected vertex to the near-most GS Z so the geometry always
    // draws on top of the world. The item-in-hand needs it: the GS cannot clear
    // the depth buffer mid-frame, so without it the hand z-clips into nearby
    // terrain. This used to disqualify a draw from the fast path entirely, which
    // meant the held item -- drawn every single frame -- went through the
    // generic emitter.
    bool forceNearZ;

    // Viewport dimensions (splitscreen): projection maps into 0..viewW/viewH;
    // the GS XYOFFSET shift positions it on screen (see ps2_set_viewport).
    float viewW;
    float viewH;

    long* debugPrims;
    // debugClipped counts frustum/outcode rejects (dead right after the VU0
    // transform); debugOffscreen counts triangles that survived the frustum,
    // paid a perspective divide, and only then failed the screen-space box.
    // They have different fixes, so they are counted apart.
    long* debugClipped;
    long* debugOffscreen;
    long* debugBackface;

    void (*setTextureClampForUv)(GSTEXTURE* texture,
                                 float u0, float v0,
                                 float u1, float v1,
                                 float u2, float v2);

    // Writes one already-decided clamp selection (see ps2_select_clamp in
    // Ps2ClipGuard.h). The batching paths must decide before they emit, because
    // gsKit_set_clamp goes straight into the DMA stream and would otherwise
    // apply to primitives already staged; they used to carry a hand-copied
    // prediction of setTextureClampForUv's logic to do that, which had drifted
    // out of sync with it. Deciding with the shared selector and applying
    // through this keeps one rule.
    void (*applyClampSel)(int mode, int ufix, int vfix);
};

bool ps2_draw_3d(const Ps2Draw3DState& state);

#endif
