#include "wii/render/WiiNativeStateSnapshot.h"

#if defined(WII_PLATFORM)

#include "wii/gx_wii.h"
#include "wii/render/WiiMatrix.h"
#include "wii/render/WiiNativeDraw.h"
#include "wii/render/WiiProjection.h"
#include "wii/render/WiiRenderTypes.h"
#include <gccore.h>

#include <algorithm>
#include <cmath>
#include <cstring>

// How glClear(GL_DEPTH_BUFFER_BIT) puts a value into the Z buffer.
//
// GX has no depth-clear command: the buffer is cleared by drawing a full-screen
// quad with the depth test forced to pass. The classic way to give that quad an
// arbitrary depth is GX_ZT_REPLACE with a 1x1 Z24X8 texture, which is what this
// backend and the OpenGX one before it both did.
//
// It is off by default because a GX_PeekZ() of the finished frame read back
// 0x000000 with it enabled -- an all-zero depth buffer, against which no
// GL_LEQUAL test can ever pass again. That failure is invisible in a menu, where
// nothing is depth-tested, and total in a world, where everything is. The
// Z-texture setup here is call-for-call identical to the OpenGX one, so why the
// replacement yields zero is still unexplained; what is avoidable is depending
// on it. The clear quad is kept safely inside the clip volume and the viewport
// Z transform supplies the requested clear depth instead.
//
// Set to 1 to go back to the Z-texture path.
#ifndef WII_CLEAR_DEPTH_ZTEXTURE
#define WII_CLEAR_DEPTH_ZTEXTURE 0
#endif

namespace
{
constexpr int kModelViewStackDepth = 32;
constexpr int kProjectionStackDepth = 4;
constexpr int kTextureStackDepth = 8;

struct NativeState
{
    WiiNativeRenderStateSnapshot snapshot{};
    int matrixMode = wiiRenderValue(WiiRenderMatrixMode::ModelView);

    Mtx modelViewStack[kModelViewStackDepth];
    int modelViewDepth = 0;
    int modelViewDropped = 0;
    Mtx44 projectionStack[kProjectionStackDepth];
    int projectionDepth = 0;
    int projectionDropped = 0;
    Mtx textureStack[2][kTextureStackDepth];
    int textureDepth[2] = {0, 0};
    int textureDropped[2] = {0, 0};
    int activeTextureUnit = 0;

    float clearColor[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    float clearDepth = 1.0f;
    int viewport[4] = {0, 0, 640, 480};

    int fogMode = wiiRenderValue(WiiRenderFogMode::Exp);
    float fogDensity = 1.0f;
    float fogStart = 0.0f;
    float fogEnd = 1.0f;
    float fogColor[4] = {0.0f, 0.0f, 0.0f, 0.0f};

    Mtx textureMatrix[2];
    bool initialized = false;
};

NativeState s_state;
#if WII_CLEAR_DEPTH_ZTEXTURE
GXTexObj s_clearDepthTexture;
u8 s_clearDepthTexels[64] ATTRIBUTE_ALIGN(32);
#endif

static float clamp01(float value)
{
    return std::max(0.0f, std::min(1.0f, value));
}

static u8 colorByte(float value)
{
    return static_cast<u8>(clamp01(value) * 255.0f + 0.5f);
}

static u8 compareToGx(unsigned int value)
{
    switch (value)
    {
        case wiiRenderValue(WiiRenderCompare::Never): return GX_NEVER;
        case wiiRenderValue(WiiRenderCompare::Less): return GX_LESS;
        case wiiRenderValue(WiiRenderCompare::Equal): return GX_EQUAL;
        case wiiRenderValue(WiiRenderCompare::LessEqual): return GX_LEQUAL;
        case wiiRenderValue(WiiRenderCompare::Greater): return GX_GREATER;
        case wiiRenderValue(WiiRenderCompare::NotEqual): return GX_NEQUAL;
        case wiiRenderValue(WiiRenderCompare::GreaterEqual): return GX_GEQUAL;
        case wiiRenderValue(WiiRenderCompare::Always): return GX_ALWAYS;
        default: return GX_ALWAYS;
    }
}

static u8 blendToGx(unsigned int value)
{
    switch (value)
    {
        case wiiRenderValue(WiiRenderBlendFactor::Zero): return GX_BL_ZERO;
        case wiiRenderValue(WiiRenderBlendFactor::One): return GX_BL_ONE;
        case wiiRenderValue(WiiRenderBlendFactor::SrcColor): return GX_BL_SRCCLR;
        case wiiRenderValue(WiiRenderBlendFactor::OneMinusSrcColor): return GX_BL_INVSRCCLR;
        case wiiRenderValue(WiiRenderBlendFactor::DstColor): return GX_BL_DSTCLR;
        case wiiRenderValue(WiiRenderBlendFactor::OneMinusDstColor): return GX_BL_INVDSTCLR;
        case wiiRenderValue(WiiRenderBlendFactor::SrcAlpha): return GX_BL_SRCALPHA;
        case wiiRenderValue(WiiRenderBlendFactor::OneMinusSrcAlpha): return GX_BL_INVSRCALPHA;
        case wiiRenderValue(WiiRenderBlendFactor::DstAlpha): return GX_BL_DSTALPHA;
        case wiiRenderValue(WiiRenderBlendFactor::OneMinusDstAlpha): return GX_BL_INVDSTALPHA;
        default: return GX_BL_ONE;
    }
}

static bool textureMatrixIdentity(int unit)
{
    static const Mtx identity = {
        {1.0f, 0.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 1.0f, 0.0f}
    };
    return unit >= 0 && unit < 2 &&
           std::memcmp(s_state.textureMatrix[unit], identity, sizeof(Mtx)) == 0;
}

static void syncTextureMatrixSnapshot(int unit)
{
    if (unit == 0)
    {
        std::memcpy(s_state.snapshot.texture0_matrix, s_state.textureMatrix[0], sizeof(Mtx));
        s_state.snapshot.texture0_identity = textureMatrixIdentity(0) ? 1 : 0;
    }
    else if (unit == 1)
    {
        std::memcpy(s_state.snapshot.texture1_matrix, s_state.textureMatrix[1], sizeof(Mtx));
        s_state.snapshot.texture1_identity = textureMatrixIdentity(1) ? 1 : 0;
    }
}

static void syncTextureMatrixSnapshots()
{
    syncTextureMatrixSnapshot(0);
    syncTextureMatrixSnapshot(1);
}

static void syncFog()
{
    wii_native_set_terrain_fog(s_state.fogMode, s_state.fogDensity,
                               s_state.fogStart, s_state.fogEnd,
                               s_state.fogColor[0], s_state.fogColor[1],
                               s_state.fogColor[2], s_state.fogColor[3]);
}

static void initializeDefaults()
{
    if (s_state.initialized)
        return;

    wiiMtxIdentity(s_state.snapshot.modelview);
    guMtx44Identity(s_state.snapshot.projection);
    wiiMtxIdentity(s_state.textureMatrix[0]);
    wiiMtxIdentity(s_state.textureMatrix[1]);
    syncTextureMatrixSnapshots();

    s_state.viewport[2] = wiigl_width();
    s_state.viewport[3] = wiigl_height();

    s_state.snapshot.current_color[0] = 1.0f;
    s_state.snapshot.current_color[1] = 1.0f;
    s_state.snapshot.current_color[2] = 1.0f;
    s_state.snapshot.current_color[3] = 1.0f;
    s_state.snapshot.current_normal[0] = 0.0f;
    s_state.snapshot.current_normal[1] = 0.0f;
    s_state.snapshot.current_normal[2] = 1.0f;
    s_state.snapshot.texture0_id = 0;
    s_state.snapshot.texture1_id = 0;
    s_state.snapshot.texture1_coord[0] = 0.0f;
    s_state.snapshot.texture1_coord[1] = 0.0f;
    s_state.snapshot.color_update = 1;
    s_state.snapshot.alpha_update = 1;
    s_state.snapshot.color_mask_r = 1;
    s_state.snapshot.color_mask_g = 1;
    s_state.snapshot.color_mask_b = 1;
    s_state.snapshot.color_mask_a = 1;
    s_state.snapshot.shade_model = wiiRenderValue(WiiRenderShadeModel::Smooth);
    s_state.snapshot.viewport[0] = s_state.viewport[0];
    s_state.snapshot.viewport[1] = s_state.viewport[1];
    s_state.snapshot.viewport[2] = s_state.viewport[2];
    s_state.snapshot.viewport[3] = s_state.viewport[3];

    s_state.snapshot.ztest = 0;
    s_state.snapshot.zwrite = 1;
    s_state.snapshot.zfunc = GX_LESS;
    s_state.snapshot.alpha_func = GX_ALWAYS;
    s_state.snapshot.src_blend = GX_BL_ONE;
    s_state.snapshot.dst_blend = GX_BL_ZERO;
    s_state.snapshot.cull_mode = wiiRenderValue(WiiRenderFace::Back);
    s_state.snapshot.front_cw = 0;

    const float globalAmbient[4] = {0.2f, 0.2f, 0.2f, 1.0f};
    const float materialAmbient[4] = {0.2f, 0.2f, 0.2f, 1.0f};
    const float materialDiffuse[4] = {0.8f, 0.8f, 0.8f, 1.0f};
    std::memcpy(s_state.snapshot.global_ambient, globalAmbient, sizeof(globalAmbient));
    std::memcpy(s_state.snapshot.material_ambient, materialAmbient, sizeof(materialAmbient));
    std::memcpy(s_state.snapshot.material_diffuse, materialDiffuse, sizeof(materialDiffuse));
    s_state.snapshot.color_material_mode = wiiRenderValue(WiiRenderColorMaterialMode::AmbientAndDiffuse);

    for (int i = 0; i < WII_NATIVE_MAX_LIGHTS; ++i)
    {
        WiiNativeLightSnapshot& light = s_state.snapshot.lights[i];
        light.position[2] = 100000.0f;
        light.position[3] = 0.0f;
        light.diffuse[3] = 1.0f;
        light.ambient[3] = 1.0f;
        light.specular[3] = 1.0f;
        if (i == 0)
            light.diffuse[0] = light.diffuse[1] = light.diffuse[2] = 1.0f;
    }

#if WII_CLEAR_DEPTH_ZTEXTURE
    GX_InitTexObj(&s_clearDepthTexture, s_clearDepthTexels, 1, 1,
                  GX_TF_Z24X8, GX_CLAMP, GX_CLAMP, GX_FALSE);
    GX_InitTexObjLOD(&s_clearDepthTexture, GX_NEAR, GX_NEAR,
                     0.0f, 0.0f, 0.0f, GX_DISABLE, GX_DISABLE, GX_ANISO_1);
#endif
    syncFog();
    s_state.initialized = true;
}

// A push that overflows its stack must not be answered by a pop that restores
// an outer level: that hands the caller somebody else's matrix and stays wrong
// for the rest of the frame. Count the dropped pushes and let the pops that
// pair with them fall through instead.
static bool acquireMatrixPush(int depth, int capacity, int& dropped)
{
    if (depth >= capacity)
    {
        ++dropped;
        return false;
    }
    return true;
}

static bool acquireMatrixPop(int depth, int& dropped)
{
    if (dropped > 0)
    {
        --dropped;
        return false;
    }
    return depth > 0;
}

static Mtx* currentMtx()
{
    if (s_state.matrixMode == wiiRenderValue(WiiRenderMatrixMode::ModelView))
        return &s_state.snapshot.modelview;
    if (s_state.matrixMode == wiiRenderValue(WiiRenderMatrixMode::Texture))
        return &s_state.textureMatrix[s_state.activeTextureUnit];
    return nullptr;
}

static void concat3x4(const Mtx transform)
{
    Mtx* current = currentMtx();
    if (!current)
        return;
    Mtx result;
    // WiiMatrix.h explains why every matrix operation here goes through
    // libogc's portable C routines instead of the paired-single ones.
    wiiMtxConcat(*current, transform, result);
    std::memcpy(*current, result, sizeof(Mtx));
    if (s_state.matrixMode == wiiRenderValue(WiiRenderMatrixMode::Texture))
        syncTextureMatrixSnapshot(s_state.activeTextureUnit);
}

static void concatProjection(const Mtx44 transform)
{
    Mtx44 result;
    wiiMtx44Concat(s_state.snapshot.projection, transform, result);
    std::memcpy(s_state.snapshot.projection, result, sizeof(Mtx44));
}

static void applyTranslation(float x, float y, float z)
{
    if (s_state.matrixMode == wiiRenderValue(WiiRenderMatrixMode::Projection))
    {
        Mtx44 transform;
        guMtx44Identity(transform);
        transform[0][3] = x;
        transform[1][3] = y;
        transform[2][3] = z;
        concatProjection(transform);
        return;
    }

    Mtx transform;
    wiiMtxTrans(transform, x, y, z);
    concat3x4(transform);
}

static void applyScale(float x, float y, float z)
{
    if (s_state.matrixMode == wiiRenderValue(WiiRenderMatrixMode::Projection))
    {
        Mtx44 transform;
        guMtx44Identity(transform);
        transform[0][0] = x;
        transform[1][1] = y;
        transform[2][2] = z;
        concatProjection(transform);
        return;
    }

    Mtx transform;
    wiiMtxScale(transform, x, y, z);
    concat3x4(transform);
}

static void applyRotation(float angle, float x, float y, float z)
{
    if (angle == 0.0f || (x == 0.0f && y == 0.0f && z == 0.0f))
        return;

    guVector axis = {x, y, z};
    Mtx rotation;
    wiiMtxRotAxisDeg(rotation, &axis, angle);

    if (s_state.matrixMode == wiiRenderValue(WiiRenderMatrixMode::Projection))
    {
        Mtx44 transform;
        guMtx44Identity(transform);
        for (int row = 0; row < 3; ++row)
            for (int column = 0; column < 4; ++column)
                transform[row][column] = rotation[row][column];
        concatProjection(transform);
        return;
    }
    concat3x4(rotation);
}

static void restoreTrackedPixelEngineState()
{
    const WiiNativeRenderStateSnapshot& state = s_state.snapshot;

    // A depth clear temporarily turns the PE into a Z-texture writer, and a
    // terrain pass imposes its own blend/cull/colour-write state.  OpenGX used
    // dirty flags to re-apply this state on the next GL draw; the native
    // backend has no such deferred state machine, so restore the logical GL
    // state immediately.  Otherwise GUI draws can inherit GX_ALWAYS/ZTexture
    // or the clear's (or the depth-only terrain pass') disabled colour writes.
    GX_SetZTexture(GX_ZT_DISABLE, GX_TF_Z24X8, 0);
    GX_SetZMode(state.ztest ? GX_TRUE : GX_FALSE, state.zfunc,
                (state.ztest && state.zwrite) ? GX_TRUE : GX_FALSE);

    const u8 alphaFunc = state.alpha_test_enabled ? state.alpha_func : GX_ALWAYS;
    const u8 alphaRef = state.alpha_test_enabled ? state.alpha_ref : 0;
    GX_SetZCompLoc(state.alpha_test_enabled ? GX_DISABLE : GX_ENABLE);
    GX_SetAlphaCompare(alphaFunc, alphaRef, GX_AOP_AND, GX_ALWAYS, 0);

    if (state.blend_enabled)
        GX_SetBlendMode(GX_BM_BLEND, state.src_blend, state.dst_blend, GX_LO_CLEAR);
    else
        GX_SetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR);

    GX_SetColorUpdate(state.color_update ? GX_TRUE : GX_FALSE);
    GX_SetAlphaUpdate(state.alpha_update ? GX_TRUE : GX_FALSE);

    GX_SetViewport(static_cast<float>(s_state.viewport[0]),
                   static_cast<float>(s_state.viewport[1]),
                   static_cast<float>(std::max(1, s_state.viewport[2])),
                   static_cast<float>(std::max(1, s_state.viewport[3])),
                   0.0f, 1.0f);
    GX_SetScissor(static_cast<u32>(std::max(0, s_state.viewport[0])),
                  static_cast<u32>(std::max(0, s_state.viewport[1])),
                  static_cast<u32>(std::max(1, s_state.viewport[2])),
                  static_cast<u32>(std::max(1, s_state.viewport[3])));

    wiiLoadProjectionFromOpenGL(state.projection, state.polygon_offset_fill != 0,
                                state.polygon_offset_units);

    // libogc declares GX_LoadPosMtxImm(Mtx, ...) with a mutable Mtx even
    // though it only uploads the matrix.  The snapshot is intentionally const
    // here, so copy it into a local mutable Mtx instead of casting const away.
    Mtx modelview;
    std::memcpy(modelview, state.modelview, sizeof(modelview));
    GX_LoadPosMtxImm(modelview, GX_PNMTX0);
    GX_SetCurrentMtx(GX_PNMTX0);
}

#if WII_CLEAR_DEPTH_ZTEXTURE
static void setDepthTexel(float depth)
{
    const u32 value = static_cast<u32>(clamp01(depth) * 16777215.0f + 0.5f);
    std::memset(s_clearDepthTexels, 0, sizeof(s_clearDepthTexels));
    s_clearDepthTexels[0] = 0xff;
    s_clearDepthTexels[1] = static_cast<u8>((value >> 16) & 0xff);
    s_clearDepthTexels[32] = static_cast<u8>((value >> 8) & 0xff);
    s_clearDepthTexels[33] = static_cast<u8>(value & 0xff);
    DCStoreRange(s_clearDepthTexels, sizeof(s_clearDepthTexels));
    GX_InvalidateTexAll();
}
#endif

static void drawClearQuad(unsigned int mask)
{
    initializeDefaults();

    const bool clearColor = (mask & WiiRenderClearMask::Color) != 0;
    const bool clearDepth = (mask & WiiRenderClearMask::Depth) != 0;
    if (!clearColor && !clearDepth)
        return;

    const float clearDepthValue = clamp01(s_state.clearDepth);
    GX_SetViewport(static_cast<float>(s_state.viewport[0]), static_cast<float>(s_state.viewport[1]),
                   static_cast<float>(s_state.viewport[2]), static_cast<float>(s_state.viewport[3]),
                   clearDepth ? clearDepthValue : 0.0f,
                   clearDepth ? clearDepthValue : 1.0f);
    GX_SetScissor(static_cast<u32>(std::max(0, s_state.viewport[0])),
                  static_cast<u32>(std::max(0, s_state.viewport[1])),
                  static_cast<u32>(std::max(1, s_state.viewport[2])),
                  static_cast<u32>(std::max(1, s_state.viewport[3])));

    Mtx identity;
    wiiMtxIdentity(identity);
    GX_LoadPosMtxImm(identity, GX_PNMTX0);
    GX_SetCurrentMtx(GX_PNMTX0);

    Mtx44 projection;
    const float left = static_cast<float>(s_state.viewport[0]) - 0.5f;
    const float top = static_cast<float>(s_state.viewport[1]) - 0.5f;
    // Keep the quad away from the far clip plane. The viewport Z range above
    // collapses depth to clearDepthValue, so the quad position no longer has to
    // encode the value written to the Z buffer.
    constexpr float clearZ = -0.5f;
    guOrtho(projection, top, top + static_cast<float>(s_state.viewport[3]),
            left, left + static_cast<float>(s_state.viewport[2]), 0.0f, 1.0f);
    GX_LoadProjectionMtx(projection, GX_ORTHOGRAPHIC);

    if (clearDepth)
    {
#if WII_CLEAR_DEPTH_ZTEXTURE
        setDepthTexel(s_state.clearDepth);
        GX_SetZMode(GX_TRUE, GX_ALWAYS, GX_TRUE);
        GX_SetZCompLoc(GX_DISABLE);
        GX_SetZTexture(GX_ZT_REPLACE, GX_TF_Z24X8, 0);
        GX_LoadTexObj(&s_clearDepthTexture, GX_TEXMAP0);
        wii_native_invalidate_texture_binding_cache();
        GX_SetNumTexGens(1);
        GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
        GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
#else
        // The viewport Z transform above forces every fragment to the requested
        // clear depth, so plain rasterisation is sufficient and the Z-texture
        // has nothing left to contribute. Keeping it would only reintroduce the
        // one part of this path whose result could not be predicted from the GX
        // calls alone.
        GX_SetZMode(GX_TRUE, GX_ALWAYS, GX_TRUE);
        GX_SetZCompLoc(GX_ENABLE);
        GX_SetZTexture(GX_ZT_DISABLE, GX_TF_Z24X8, 0);
        GX_SetNumTexGens(0);
        GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORDNULL, GX_TEXMAP_NULL, GX_COLOR0A0);
#endif
    }
    else
    {
        GX_SetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
        GX_SetZTexture(GX_ZT_DISABLE, GX_TF_Z24X8, 0);
        GX_SetNumTexGens(0);
        GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORDNULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    }

    GX_SetColorUpdate((clearColor && s_state.snapshot.color_update) ? GX_TRUE : GX_FALSE);
    GX_SetAlphaUpdate((clearColor && s_state.snapshot.alpha_update) ? GX_TRUE : GX_FALSE);
    GX_SetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_COPY);
    GX_SetCullMode(GX_CULL_NONE);
    GX_SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
    GX_SetNumChans(1);
    GX_SetNumIndStages(0);
    GX_SetNumTevStages(1);
    GX_SetTevDirect(GX_TEVSTAGE0);
    GX_SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_RASC);
    GX_SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_RASA);
    GX_SetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GX_SetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GX_SetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_VTX, GX_SRC_VTX,
                   GX_LIGHTNULL, GX_DF_NONE, GX_AF_NONE);
    GX_SetFog(GX_FOG_NONE, 0.0f, 1.0f, 0.1f, 1.0f, GXColor{0, 0, 0, 0});

    GX_ClearVtxDesc();
    GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
    GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
#if WII_CLEAR_DEPTH_ZTEXTURE
    // Keep TEX0 only for the disabled diagnostic fallback, where the Z-texture
    // path still needs a coordinate to sample its single texel.
    GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_U8, 0);
#endif
    GX_InvVtxCache();

    const GXColor color = {colorByte(s_state.clearColor[0]), colorByte(s_state.clearColor[1]),
                           colorByte(s_state.clearColor[2]), colorByte(s_state.clearColor[3])};
    const float width = static_cast<float>(std::max(1, s_state.viewport[2]));
    const float height = static_cast<float>(std::max(1, s_state.viewport[3]));
    GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
    GX_Position3f32(0.0f, 0.0f, clearZ);
    GX_Color4u8(color.r, color.g, color.b, color.a);
#if WII_CLEAR_DEPTH_ZTEXTURE
    GX_TexCoord2u8(0, 0);
#endif
    GX_Position3f32(0.0f, height, clearZ);
    GX_Color4u8(color.r, color.g, color.b, color.a);
#if WII_CLEAR_DEPTH_ZTEXTURE
    GX_TexCoord2u8(0, 1);
#endif
    GX_Position3f32(width, height, clearZ);
    GX_Color4u8(color.r, color.g, color.b, color.a);
#if WII_CLEAR_DEPTH_ZTEXTURE
    GX_TexCoord2u8(1, 1);
#endif
    GX_Position3f32(width, 0.0f, clearZ);
    GX_Color4u8(color.r, color.g, color.b, color.a);
#if WII_CLEAR_DEPTH_ZTEXTURE
    GX_TexCoord2u8(1, 0);
#endif
    GX_End();

    // Do not leak the clear pass' PE/matrix state into the next native draw.
    restoreTrackedPixelEngineState();
}

static void matrixToGl(const Mtx matrix, float* out)
{
    for (int row = 0; row < 4; ++row)
        for (int column = 0; column < 4; ++column)
            out[column * 4 + row] = (row == 3) ? (column == 3 ? 1.0f : 0.0f)
                : matrix[row][column];
}

static void matrix44ToGl(const Mtx44 matrix, float* out)
{
    for (int row = 0; row < 4; ++row)
        for (int column = 0; column < 4; ++column)
            out[column * 4 + row] = matrix[row][column];
}
}

extern "C" void wii_native_state_initialize(void)
{
    initializeDefaults();
}

extern "C" int wii_gx_get_native_state_snapshot(WiiNativeRenderStateSnapshot* outState)
{
    initializeDefaults();
    if (!outState)
        return 0;
    syncTextureMatrixSnapshots();
    *outState = s_state.snapshot;
    return 1;
}

extern "C" void wii_gx_get_native_modelview(float outModelView[3][4])
{
    initializeDefaults();
    if (outModelView)
        std::memcpy(outModelView, s_state.snapshot.modelview, sizeof(Mtx));
}

extern "C" void wii_gx_invalidate_after_native_draw(void)
{
    // Native GX draws consume this state directly, so there is no compatibility cache to invalidate.
}

extern "C" void wii_gx_native_restore_pixel_engine(void)
{
    initializeDefaults();
    restoreTrackedPixelEngineState();
}

extern "C" int wii_gx_native_modelview_push(void)
{
    initializeDefaults();
    if (!acquireMatrixPush(s_state.modelViewDepth, kModelViewStackDepth, s_state.modelViewDropped))
        return 0;
    std::memcpy(s_state.modelViewStack[s_state.modelViewDepth++], s_state.snapshot.modelview, sizeof(Mtx));
    return 1;
}

extern "C" int wii_gx_native_modelview_pop(void)
{
    initializeDefaults();
    if (!acquireMatrixPop(s_state.modelViewDepth, s_state.modelViewDropped))
        return 0;
    std::memcpy(s_state.snapshot.modelview, s_state.modelViewStack[--s_state.modelViewDepth], sizeof(Mtx));
    return 1;
}

extern "C" void wii_gx_native_modelview_translate(float x, float y, float z)
{
    initializeDefaults();
    wiiMtxApplyTrans(s_state.snapshot.modelview, s_state.snapshot.modelview, x, y, z);
}

extern "C" void wii_gx_native_modelview_rotate(float angle, float x, float y, float z)
{
    initializeDefaults();
    if (angle == 0.0f || (x == 0.0f && y == 0.0f && z == 0.0f))
        return;
    guVector axis = {x, y, z};
    Mtx rotation;
    wiiMtxRotAxisDeg(rotation, &axis, angle);
    Mtx result;
    wiiMtxConcat(s_state.snapshot.modelview, rotation, result);
    std::memcpy(s_state.snapshot.modelview, result, sizeof(Mtx));
}

extern "C" void wii_gx_native_modelview_scale(float x, float y, float z)
{
    initializeDefaults();
    wiiMtxApplyScale(s_state.snapshot.modelview, s_state.snapshot.modelview, x, y, z);
}

extern "C" void wii_gx_native_enable(unsigned int cap)
{
    initializeDefaults();
    switch (cap)
    {
        case wiiRenderValue(WiiRenderCapability::Texture2D): s_state.snapshot.texture_enabled |= (1u << s_state.activeTextureUnit); break;
        case wiiRenderValue(WiiRenderCapability::ColorMaterial): s_state.snapshot.color_material_enabled = 1; break;
        case wiiRenderValue(WiiRenderCapability::CullFace): s_state.snapshot.cull_enabled = 1; break;
        case wiiRenderValue(WiiRenderCapability::AlphaTest): s_state.snapshot.alpha_test_enabled = 1; break;
        case wiiRenderValue(WiiRenderCapability::Blend): s_state.snapshot.blend_enabled = 1; break;
        case wiiRenderValue(WiiRenderCapability::DepthTest): s_state.snapshot.ztest = 1; break;
        case wiiRenderValue(WiiRenderCapability::Fog): s_state.snapshot.fog_enabled = 1; break;
        case wiiRenderValue(WiiRenderCapability::Lighting): s_state.snapshot.lighting_enabled = 1; break;
        case wiiRenderValue(WiiRenderCapability::Normalize): s_state.snapshot.normalize_normals = 1; break;
        case wiiRenderValue(WiiRenderCapability::RescaleNormal): s_state.snapshot.rescale_normals = 1; break;
        case wiiRenderValue(WiiRenderCapability::Light0):
        case wiiRenderValue(WiiRenderCapability::Light1): s_state.snapshot.lights[cap - wiiRenderValue(WiiRenderCapability::Light0)].enabled = 1; break;
        case wiiRenderValue(WiiRenderCapability::PolygonOffsetFill): s_state.snapshot.polygon_offset_fill = 1; break;
        default: break;
    }
}

extern "C" void wii_gx_native_disable(unsigned int cap)
{
    initializeDefaults();
    switch (cap)
    {
        case wiiRenderValue(WiiRenderCapability::Texture2D): s_state.snapshot.texture_enabled &= ~(1u << s_state.activeTextureUnit); break;
        case wiiRenderValue(WiiRenderCapability::ColorMaterial): s_state.snapshot.color_material_enabled = 0; break;
        case wiiRenderValue(WiiRenderCapability::CullFace): s_state.snapshot.cull_enabled = 0; break;
        case wiiRenderValue(WiiRenderCapability::AlphaTest): s_state.snapshot.alpha_test_enabled = 0; break;
        case wiiRenderValue(WiiRenderCapability::Blend): s_state.snapshot.blend_enabled = 0; break;
        case wiiRenderValue(WiiRenderCapability::DepthTest): s_state.snapshot.ztest = 0; break;
        case wiiRenderValue(WiiRenderCapability::Fog): s_state.snapshot.fog_enabled = 0; break;
        case wiiRenderValue(WiiRenderCapability::Lighting): s_state.snapshot.lighting_enabled = 0; break;
        case wiiRenderValue(WiiRenderCapability::Normalize): s_state.snapshot.normalize_normals = 0; break;
        case wiiRenderValue(WiiRenderCapability::RescaleNormal): s_state.snapshot.rescale_normals = 0; break;
        case wiiRenderValue(WiiRenderCapability::Light0):
        case wiiRenderValue(WiiRenderCapability::Light1): s_state.snapshot.lights[cap - wiiRenderValue(WiiRenderCapability::Light0)].enabled = 0; break;
        case wiiRenderValue(WiiRenderCapability::PolygonOffsetFill): s_state.snapshot.polygon_offset_fill = 0; break;
        default: break;
    }
}

extern "C" void wii_gx_native_blend_func(unsigned int src, unsigned int dst)
{
    initializeDefaults();
    s_state.snapshot.src_blend = blendToGx(src);
    s_state.snapshot.dst_blend = blendToGx(dst);
}

extern "C" void wii_gx_native_depth_mask(unsigned char enabled)
{
    initializeDefaults();
    s_state.snapshot.zwrite = enabled ? 1 : 0;
}

extern "C" void wii_gx_native_depth_func(unsigned int func)
{
    initializeDefaults();
    s_state.snapshot.zfunc = compareToGx(func);
}

extern "C" void wii_gx_native_alpha_func(unsigned int func, float ref)
{
    initializeDefaults();
    s_state.snapshot.alpha_func = compareToGx(func);
    s_state.snapshot.alpha_ref = colorByte(ref);
}

extern "C" void wii_gx_native_cull_face(unsigned int mode)
{
    initializeDefaults();
    s_state.snapshot.cull_mode = static_cast<int>(mode);
}

extern "C" void wii_gx_native_color4f(float r, float g, float b, float a)
{
    initializeDefaults();
    s_state.snapshot.current_color[0] = r;
    s_state.snapshot.current_color[1] = g;
    s_state.snapshot.current_color[2] = b;
    s_state.snapshot.current_color[3] = a;
}

extern "C" void wii_gx_native_color3f(float r, float g, float b)
{
    wii_gx_native_color4f(r, g, b, 1.0f);
}

extern "C" void wii_gx_native_color_mask(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    initializeDefaults();
    s_state.snapshot.color_mask_r = r ? 1 : 0;
    s_state.snapshot.color_mask_g = g ? 1 : 0;
    s_state.snapshot.color_mask_b = b ? 1 : 0;
    s_state.snapshot.color_mask_a = a ? 1 : 0;
    s_state.snapshot.color_update = (r || g || b) ? 1 : 0;
    s_state.snapshot.alpha_update = a ? 1 : 0;
}

extern "C" void wii_gx_native_fogf(unsigned int pname, float value)
{
    initializeDefaults();
    switch (pname)
    {
        case wiiRenderValue(WiiRenderFogParameter::Mode): s_state.fogMode = static_cast<int>(value); break;
        case wiiRenderValue(WiiRenderFogParameter::Density): s_state.fogDensity = value; break;
        case wiiRenderValue(WiiRenderFogParameter::Start): s_state.fogStart = value; break;
        case wiiRenderValue(WiiRenderFogParameter::End): s_state.fogEnd = value; break;
        default: return;
    }
    syncFog();
}

extern "C" void wii_gx_native_fogi(unsigned int pname, int value)
{
    wii_gx_native_fogf(pname, static_cast<float>(value));
}

extern "C" void wii_gx_native_fogfv(unsigned int pname, const float* values)
{
    initializeDefaults();
    if (!values)
        return;
    if (pname == wiiRenderValue(WiiRenderFogParameter::Color))
    {
        std::memcpy(s_state.fogColor, values, sizeof(s_state.fogColor));
        syncFog();
        return;
    }
    wii_gx_native_fogf(pname, values[0]);
}

extern "C" void wii_gx_native_lightfv(unsigned int light, unsigned int pname, const float* values)
{
    initializeDefaults();
    if (!values || light < wiiRenderValue(WiiRenderCapability::Light0) || light >= wiiRenderValue(WiiRenderCapability::Light0) + WII_NATIVE_MAX_LIGHTS)
        return;
    WiiNativeLightSnapshot& target = s_state.snapshot.lights[light - wiiRenderValue(WiiRenderCapability::Light0)];
    switch (pname)
    {
        case wiiRenderValue(WiiRenderLightParameter::Position):
        {
            guVector source = {values[0], values[1], values[2]};
            guVector transformed;
            if (values[3] == 0.0f)
            {
                wiiVecMultiplySR(s_state.snapshot.modelview, &source, &transformed);
                const float lengthSquared = transformed.x * transformed.x + transformed.y * transformed.y + transformed.z * transformed.z;
                if (lengthSquared > 1.0e-12f)
                {
                    const float inverseLength = 1.0f / std::sqrt(lengthSquared);
                    transformed.x *= inverseLength;
                    transformed.y *= inverseLength;
                    transformed.z *= inverseLength;
                }
                target.position[0] = transformed.x * 100000.0f;
                target.position[1] = transformed.y * 100000.0f;
                target.position[2] = transformed.z * 100000.0f;
            }
            else
            {
                wiiVecMultiply(s_state.snapshot.modelview, &source, &transformed);
                target.position[0] = transformed.x;
                target.position[1] = transformed.y;
                target.position[2] = transformed.z;
            }
            target.position[3] = values[3];
            break;
        }
        case wiiRenderValue(WiiRenderLightParameter::Ambient): std::memcpy(target.ambient, values, sizeof(target.ambient)); break;
        case wiiRenderValue(WiiRenderLightParameter::Diffuse): std::memcpy(target.diffuse, values, sizeof(target.diffuse)); break;
        case wiiRenderValue(WiiRenderLightParameter::Specular): std::memcpy(target.specular, values, sizeof(target.specular)); break;
        default: break;
    }
}

extern "C" void wii_gx_native_light_modelfv(unsigned int pname, const float* values)
{
    initializeDefaults();
    if (pname == wiiRenderValue(WiiRenderLightModelParameter::Ambient) && values)
        std::memcpy(s_state.snapshot.global_ambient, values, sizeof(s_state.snapshot.global_ambient));
}

extern "C" void wii_gx_native_color_material(unsigned int, unsigned int mode)
{
    initializeDefaults();
    s_state.snapshot.color_material_mode = static_cast<int>(mode);
}

extern "C" void wii_gx_native_shade_model(unsigned int mode)
{
    initializeDefaults();
    if (mode == wiiRenderValue(WiiRenderShadeModel::Flat) ||
        mode == wiiRenderValue(WiiRenderShadeModel::Smooth))
        s_state.snapshot.shade_model = mode;
}

extern "C" void wii_gx_native_matrix_mode(unsigned int mode)
{
    initializeDefaults();
    if (mode == wiiRenderValue(WiiRenderMatrixMode::ModelView) || mode == wiiRenderValue(WiiRenderMatrixMode::Projection) || mode == wiiRenderValue(WiiRenderMatrixMode::Texture))
        s_state.matrixMode = static_cast<int>(mode);
}

extern "C" void wii_gx_native_load_identity(void)
{
    initializeDefaults();
    if (s_state.matrixMode == wiiRenderValue(WiiRenderMatrixMode::ModelView))
        wiiMtxIdentity(s_state.snapshot.modelview);
    else if (s_state.matrixMode == wiiRenderValue(WiiRenderMatrixMode::Projection))
        guMtx44Identity(s_state.snapshot.projection);
    else if (s_state.matrixMode == wiiRenderValue(WiiRenderMatrixMode::Texture))
    {
        wiiMtxIdentity(s_state.textureMatrix[s_state.activeTextureUnit]);
        syncTextureMatrixSnapshot(s_state.activeTextureUnit);
    }
}

extern "C" void wii_gx_native_matrix_push(void)
{
    initializeDefaults();
    if (s_state.matrixMode == wiiRenderValue(WiiRenderMatrixMode::ModelView))
        wii_gx_native_modelview_push();
    else if (s_state.matrixMode == wiiRenderValue(WiiRenderMatrixMode::Projection))
    {
        if (acquireMatrixPush(s_state.projectionDepth, kProjectionStackDepth, s_state.projectionDropped))
            std::memcpy(s_state.projectionStack[s_state.projectionDepth++], s_state.snapshot.projection, sizeof(Mtx44));
    }
    else if (s_state.matrixMode == wiiRenderValue(WiiRenderMatrixMode::Texture))
    {
        const int unit = s_state.activeTextureUnit;
        if (acquireMatrixPush(s_state.textureDepth[unit], kTextureStackDepth, s_state.textureDropped[unit]))
            std::memcpy(s_state.textureStack[unit][s_state.textureDepth[unit]++],
                        s_state.textureMatrix[unit], sizeof(Mtx));
    }
}

extern "C" void wii_gx_native_matrix_pop(void)
{
    initializeDefaults();
    if (s_state.matrixMode == wiiRenderValue(WiiRenderMatrixMode::ModelView))
        wii_gx_native_modelview_pop();
    else if (s_state.matrixMode == wiiRenderValue(WiiRenderMatrixMode::Projection))
    {
        if (acquireMatrixPop(s_state.projectionDepth, s_state.projectionDropped))
            std::memcpy(s_state.snapshot.projection, s_state.projectionStack[--s_state.projectionDepth], sizeof(Mtx44));
    }
    else if (s_state.matrixMode == wiiRenderValue(WiiRenderMatrixMode::Texture))
    {
        const int unit = s_state.activeTextureUnit;
        if (acquireMatrixPop(s_state.textureDepth[unit], s_state.textureDropped[unit]))
        {
            std::memcpy(s_state.textureMatrix[unit],
                        s_state.textureStack[unit][--s_state.textureDepth[unit]], sizeof(Mtx));
            syncTextureMatrixSnapshot(unit);
        }
    }
}

extern "C" void wii_gx_native_translatef(float x, float y, float z)
{
    initializeDefaults();
    applyTranslation(x, y, z);
}

extern "C" void wii_gx_native_rotatef(float angle, float x, float y, float z)
{
    initializeDefaults();
    applyRotation(angle, x, y, z);
}

extern "C" void wii_gx_native_scalef(float x, float y, float z)
{
    initializeDefaults();
    if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z))
        return;
    applyScale(x, y, z);
}

extern "C" void wii_gx_native_scaled(double x, double y, double z)
{
    wii_gx_native_scalef(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
}

extern "C" void wii_gx_native_frustum(double left, double right, double bottom, double top,
                                        double nearValue, double farValue)
{
    initializeDefaults();
    Mtx44 matrix{};
    const float width = static_cast<float>(right - left);
    const float height = static_cast<float>(top - bottom);
    const float depth = static_cast<float>(farValue - nearValue);
    matrix[0][0] = static_cast<float>(2.0 * nearValue) / width;
    matrix[0][2] = static_cast<float>(right + left) / width;
    matrix[1][1] = static_cast<float>(2.0 * nearValue) / height;
    matrix[1][2] = static_cast<float>(top + bottom) / height;
    matrix[2][2] = -static_cast<float>(farValue + nearValue) / depth;
    matrix[2][3] = -static_cast<float>(2.0 * farValue * nearValue) / depth;
    matrix[3][2] = -1.0f;
    if (s_state.matrixMode == wiiRenderValue(WiiRenderMatrixMode::Projection))
        concatProjection(matrix);
}

extern "C" void wii_gx_native_ortho(double left, double right, double bottom, double top,
                                      double nearValue, double farValue)
{
    initializeDefaults();
    Mtx44 matrix{};
    matrix[0][0] = static_cast<float>(2.0 / (right - left));
    matrix[1][1] = static_cast<float>(2.0 / (top - bottom));
    matrix[2][2] = static_cast<float>(-2.0 / (farValue - nearValue));
    matrix[3][3] = 1.0f;
    matrix[0][3] = -static_cast<float>((right + left) / (right - left));
    matrix[1][3] = -static_cast<float>((top + bottom) / (top - bottom));
    matrix[2][3] = -static_cast<float>((farValue + nearValue) / (farValue - nearValue));
    if (s_state.matrixMode == wiiRenderValue(WiiRenderMatrixMode::Projection))
        concatProjection(matrix);
}

extern "C" void wii_gx_native_normal3f(float x, float y, float z)
{
    initializeDefaults();
    s_state.snapshot.current_normal[0] = x;
    s_state.snapshot.current_normal[1] = y;
    s_state.snapshot.current_normal[2] = z;
}

extern "C" void wii_gx_native_clear(unsigned int mask)
{
    drawClearQuad(mask);
}

extern "C" void wii_gx_native_clear_color(float r, float g, float b, float a)
{
    initializeDefaults();
    s_state.clearColor[0] = r;
    s_state.clearColor[1] = g;
    s_state.clearColor[2] = b;
    s_state.clearColor[3] = a;
}

extern "C" void wii_gx_native_clear_depth(double depth)
{
    initializeDefaults();
    s_state.clearDepth = clamp01(static_cast<float>(depth));
}

extern "C" void wii_gx_native_polygon_offset(float, float units)
{
    initializeDefaults();
    s_state.snapshot.polygon_offset_units = units;
}

extern "C" void wii_gx_native_line_width(float width)
{
    initializeDefaults();
    const unsigned int gxWidth = static_cast<unsigned int>(std::max(1.0f, std::min(255.0f, width * 16.0f)));
    GX_SetLineWidth(gxWidth, GX_TO_ZERO);
}

extern "C" void wii_gx_native_viewport(int x, int y, int width, int height)
{
    initializeDefaults();
    s_state.viewport[0] = x;
    s_state.viewport[1] = y;
    s_state.viewport[2] = std::max(1, width);
    s_state.viewport[3] = std::max(1, height);
    s_state.snapshot.viewport[0] = s_state.viewport[0];
    s_state.snapshot.viewport[1] = s_state.viewport[1];
    s_state.snapshot.viewport[2] = s_state.viewport[2];
    s_state.snapshot.viewport[3] = s_state.viewport[3];
    GX_SetViewport(static_cast<float>(x), static_cast<float>(y),
                   static_cast<float>(s_state.viewport[2]), static_cast<float>(s_state.viewport[3]),
                   0.0f, 1.0f);
    GX_SetScissor(static_cast<u32>(std::max(0, x)), static_cast<u32>(std::max(0, y)),
                  static_cast<u32>(s_state.viewport[2]), static_cast<u32>(s_state.viewport[3]));
}

extern "C" void wii_gx_native_get_floatv(unsigned int pname, float* values)
{
    initializeDefaults();
    if (!values)
        return;
    if (pname == wiiRenderValue(WiiRenderMatrixQuery::ModelView))
        matrixToGl(s_state.snapshot.modelview, values);
    else if (pname == wiiRenderValue(WiiRenderMatrixQuery::Projection))
        matrix44ToGl(s_state.snapshot.projection, values);
    else if (pname == wiiRenderValue(WiiRenderMatrixQuery::Texture))
        matrixToGl(s_state.textureMatrix[s_state.activeTextureUnit], values);
}

extern "C" const unsigned char* wii_gx_native_get_string(unsigned int name)
{
    switch (name)
    {
        case wiiRenderValue(WiiRenderStringQuery::Vendor): return reinterpret_cast<const unsigned char*>("Nintendo/libogc");
        case wiiRenderValue(WiiRenderStringQuery::Renderer): return reinterpret_cast<const unsigned char*>("Beta++ native GX");
        case wiiRenderValue(WiiRenderStringQuery::Version): return reinterpret_cast<const unsigned char*>("GX native");
        case wiiRenderValue(WiiRenderStringQuery::Extensions): return reinterpret_cast<const unsigned char*>("");
        default: return reinterpret_cast<const unsigned char*>("");
    }
}

extern "C" unsigned int wii_gx_native_get_error(void)
{
    return WII_RENDER_NO_ERROR;
}

extern "C" void wii_gx_native_hint(unsigned int, unsigned int)
{
}

extern "C" void wii_gx_native_set_active_texture_unit(int unit)
{
    initializeDefaults();
    if (unit >= 0 && unit < 2)
        s_state.activeTextureUnit = unit;
}

extern "C" void wii_gx_native_set_multi_texture_coord(int unit, float u, float v)
{
    initializeDefaults();
    if (unit == 1)
    {
        s_state.snapshot.texture1_coord[0] = u;
        s_state.snapshot.texture1_coord[1] = v;
    }
}

extern "C" void wii_gx_native_set_bound_texture_id(unsigned int texture)
{
    initializeDefaults();
    if (s_state.activeTextureUnit == 1)
        s_state.snapshot.texture1_id = static_cast<int>(texture);
    else
        s_state.snapshot.texture0_id = static_cast<int>(texture);
}

#endif
