#ifdef PS2_PLATFORM

#include "platform/Log.h"
#include <gsKit.h>
#include <dmaKit.h>
#include <gsPrimitive.h>
#include <gsTexture.h>
#include <gsMisc.h>
#include <gsCore.h>
#include <gsInline.h>
#include <libvux.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <malloc.h>
#include <cstdio>

// Unconditional: the generic native array path uses ClipVert, the outcodes and
// the clamp selector from this header whether or not the fast path is built.
#include "ps2/render/Ps2ClipGuard.h"
#include "ps2/render/Ps2MatrixStack.h"
#include "ps2/render/Ps2NativeDraw.h"
#include "ps2/render/Ps2Draw2D.h"
#include "ps2/render/Ps2RenderState.h"
#include "ps2/render/Ps2TextureGs.h"
#include "ps2/render/Ps2RenderApi.h"
#include "ps2/render/Ps2TerrainRenderer.h"
#ifdef PS2_ENABLE_VU1_TERRAIN
#include "ps2/render/Ps2Vu1Terrain.h"
#endif
#include "ps2/render/Ps2Draw3D.h"

#include "ps2/render/Ps2RenderBackend.h"
#include "ps2/render/Ps2GsQueue.h"
#include "ps2/render/Ps2RenderTypes.h"
#include "ps2/render/Ps2Viewport.h"
#include "ps2/render/Ps2ProjectionState.h"


namespace {
constexpr unsigned int kPrimPoints = 0x0000;
constexpr unsigned int kPrimLines = 0x0001;
constexpr unsigned int kPrimLineLoop = 0x0002;
constexpr unsigned int kPrimLineStrip = 0x0003;
constexpr unsigned int kPrimTriangles = 0x0004;
constexpr unsigned int kPrimTriangleStrip = 0x0005;
constexpr unsigned int kPrimTriangleFan = 0x0006;
constexpr unsigned int kPrimQuads = 0x0007;
constexpr unsigned int kTypeByte = 0x1400;
constexpr unsigned int kTypeFloat = 0x1406;
constexpr unsigned int kCompareNever = ps2RenderValue(Ps2RenderCompare::Never);
constexpr unsigned int kCompareLess = ps2RenderValue(Ps2RenderCompare::Less);
constexpr unsigned int kCompareEqual = ps2RenderValue(Ps2RenderCompare::Equal);
constexpr unsigned int kCompareLessEqual = ps2RenderValue(Ps2RenderCompare::LessEqual);
constexpr unsigned int kCompareGreater = ps2RenderValue(Ps2RenderCompare::Greater);
constexpr unsigned int kCompareNotEqual = ps2RenderValue(Ps2RenderCompare::NotEqual);
constexpr unsigned int kCompareGreaterEqual = ps2RenderValue(Ps2RenderCompare::GreaterEqual);
constexpr unsigned int kCompareAlways = ps2RenderValue(Ps2RenderCompare::Always);
constexpr unsigned int kBlendZero = ps2RenderValue(Ps2RenderBlendFactor::Zero);
constexpr unsigned int kBlendOne = ps2RenderValue(Ps2RenderBlendFactor::One);
constexpr unsigned int kBlendSrcColor = ps2RenderValue(Ps2RenderBlendFactor::SrcColor);
constexpr unsigned int kBlendOneMinusSrcColor = ps2RenderValue(Ps2RenderBlendFactor::OneMinusSrcColor);
constexpr unsigned int kBlendSrcAlpha = ps2RenderValue(Ps2RenderBlendFactor::SrcAlpha);
constexpr unsigned int kBlendOneMinusSrcAlpha = ps2RenderValue(Ps2RenderBlendFactor::OneMinusSrcAlpha);
constexpr unsigned int kBlendDstColor = ps2RenderValue(Ps2RenderBlendFactor::DstColor);
constexpr unsigned int kBlendOneMinusDstColor = ps2RenderValue(Ps2RenderBlendFactor::OneMinusDstColor);
constexpr unsigned int kBlendEquationAdd = 0x8006;
constexpr unsigned int kBlendEquationSubtract = 0x800A;
constexpr unsigned int kBlendEquationReverseSubtract = 0x800B;
constexpr unsigned int kCapFog = ps2RenderValue(Ps2RenderCapability::Fog);
constexpr unsigned int kCapLighting = ps2RenderValue(Ps2RenderCapability::Lighting);
constexpr unsigned int kCapDepthTest = ps2RenderValue(Ps2RenderCapability::DepthTest);
constexpr unsigned int kCapAlphaTest = ps2RenderValue(Ps2RenderCapability::AlphaTest);
constexpr unsigned int kCapBlend = ps2RenderValue(Ps2RenderCapability::Blend);
constexpr unsigned int kCapCullFace = ps2RenderValue(Ps2RenderCapability::CullFace);
constexpr unsigned int kCapTexture2D = ps2RenderValue(Ps2RenderCapability::Texture2D);
constexpr unsigned int kCapNormalize = ps2RenderValue(Ps2RenderCapability::Normalize);
constexpr unsigned int kCapRescaleNormal = ps2RenderValue(Ps2RenderCapability::RescaleNormal);
constexpr unsigned int kCapColorMaterial = ps2RenderValue(Ps2RenderCapability::ColorMaterial);
constexpr unsigned int kCapLight0 = ps2RenderValue(Ps2RenderCapability::Light0);
constexpr unsigned int kCapLight1 = ps2RenderValue(Ps2RenderCapability::Light1);
constexpr unsigned int kCapPolygonOffsetFill = ps2RenderValue(Ps2RenderCapability::PolygonOffsetFill);
constexpr unsigned int kArrayVertex = 0x8074;
constexpr unsigned int kArrayNormal = 0x8075;
constexpr unsigned int kArrayColor = 0x8076;
constexpr unsigned int kArrayTexCoord = 0x8078;
constexpr unsigned int kFogStart = ps2RenderValue(Ps2RenderFogParameter::Start);
constexpr unsigned int kFogEnd = ps2RenderValue(Ps2RenderFogParameter::End);
constexpr unsigned int kFogMode = ps2RenderValue(Ps2RenderFogParameter::Mode);
constexpr unsigned int kFogColor = ps2RenderValue(Ps2RenderFogParameter::Color);
constexpr unsigned int kFogExp = ps2RenderValue(Ps2RenderFogMode::Exp);
constexpr unsigned int kFogExp2 = ps2RenderValue(Ps2RenderFogMode::Exp2);
constexpr unsigned int kFogLinear = ps2RenderValue(Ps2RenderFogMode::Linear);
constexpr unsigned int kFaceCW = 0x0900;
constexpr unsigned int kFaceCCW = 0x0901;
constexpr unsigned int kFaceFront = ps2RenderValue(Ps2RenderFace::Front);
constexpr unsigned int kFaceBack = ps2RenderValue(Ps2RenderFace::Back);
constexpr unsigned int kShadeFlat = ps2RenderValue(Ps2RenderShadeModel::Flat);
constexpr unsigned int kShadeSmooth = ps2RenderValue(Ps2RenderShadeModel::Smooth);
constexpr unsigned int kClearColorBit = Ps2RenderClearMask::Color;
constexpr unsigned int kClearDepthBit = Ps2RenderClearMask::Depth;
constexpr unsigned int kFilterNearestMipmapLinear = 0x2702;
constexpr unsigned int kTextureMagFilter = 0x2800;
constexpr unsigned int kTextureMinFilter = 0x2801;
constexpr unsigned int kTextureWrapS = 0x2802;
constexpr unsigned int kTextureWrapT = 0x2803;
constexpr unsigned int kTextureClamp = 0x2900;
constexpr unsigned int kMatrixModelView = ps2RenderValue(Ps2RenderMatrixMode::ModelView);
constexpr unsigned int kMatrixProjection = ps2RenderValue(Ps2RenderMatrixMode::Projection);
constexpr unsigned int kLightAmbient = ps2RenderValue(Ps2RenderLightParameter::Ambient);
constexpr unsigned int kLightDiffuse = ps2RenderValue(Ps2RenderLightParameter::Diffuse);
constexpr unsigned int kLightSpecular = ps2RenderValue(Ps2RenderLightParameter::Specular);
constexpr unsigned int kLightPosition = ps2RenderValue(Ps2RenderLightParameter::Position);
constexpr unsigned int kLightModelAmbient = 0x0B53;
constexpr unsigned int kAmbientAndDiffuse = ps2RenderValue(Ps2RenderColorMaterialMode::AmbientAndDiffuse);
} // namespace

// ---- Matrix stack ----

typedef float Mat4[16]; // column-major, OpenGL convention

// Copy Mat4 into VU_MATRIX for Vu0ApplyMatrix.
// Vu0ApplyMatrix loads vf1..vf4 as consecutive 16-byte blocks and computes
//   out.x = vf1.x*vx + vf2.x*vy + vf3.x*vz + vf4.x*vw
// With column-major Mat4: mat4[0..3]=col0, mat4[4..7]=col1, etc.
//   vf1.x = mat4[0] = M00, vf2.x = mat4[4] = M01 → out.x = M00*vx+M01*vy+... ✓
// Do NOT transpose — just memcpy.
static inline VU_MATRIX mat4_to_vu(const Mat4 m) {
    VU_MATRIX out;
    __builtin_memcpy(&out, m, 16 * sizeof(float));
    return out;
}


// gsGlobal created in main_ps2.cpp
extern GSGLOBAL* gsGlobal;

// ---- GS depth state (TEST.ZTST + ZBUF.ZMSK) ----
//
// The GS keeps depth behaviour in two registers gsKit never touches after
// init: TEST.ZTST (compare func; there is no "test off" bit, only ALWAYS)
// and ZBUF.ZMSK (Z write mask). The GL wrapper used to fake GL depth state
// by pinning GUI vertices to Z=0xFFFF, which breaks anything that needs a
// real depth buffer mid-frame (inventory player preview, item-in-hand after
// glClear(kClearDepthBit), 3D item icons). Manage both registers here,
// cached so a packet is only queued when the value actually changes.
static u8 s_gsZtst = 0xFF; // last emitted TEST.ZTST (0xFF = unknown/resync)
static u8 s_gsZmsk = 0xFF; // last emitted ZBUF.ZMSK (0xFF = unknown/resync)
static bool s_gsFrameMaskValid = false;
static u32 s_gsFrameMask = 0;
static u32 s_gsFrameFbp = 0;  // last emitted FRAME.FBP, in GS words

// Validity flags for the other cached GS state. Declared here with the depth
// caches because glClear (below) has to drop all of them together — gsKit_clear
// re-emits TEST and the PRIM/ALPHA registers behind every one of these caches.
// The values they guard live next to their apply functions further down.
static bool s_primAlphaEnableValid = false;
static bool s_blendAlphaValid = false;
static bool s_alphaTestValid = false;

// Queue one A+D register write. gsKit_heap_alloc reserves one extra qword
// for the GIFTAG when the type is GIF_AD (see _gsKit_heap_alloc).
static void ps2_gs_write_reg(u64 data, u64 addr) {
    if (!gsGlobal || gsGlobal->CurQueue == nullptr) return;
    u64* p = (u64*)gsKit_heap_alloc(gsGlobal, 1, 16, GIF_AD);
    if (!p) return;
    *p++ = GIF_TAG_AD(1);
    *p++ = GIF_AD; // REGS descriptor: A+D
    *p++ = data;
    *p++ = addr;
}

static void ps2_set_ztst(u8 ztst) {
    if (!gsGlobal || !gsGlobal->Test) return;
    if (s_gsZtst == ztst) return;
    // Keep the gsKit-side struct in sync so gsKit_set_test (alpha test path,
    // gsKit_clear) re-emits the same ZTST instead of clobbering it.
    gsGlobal->Test->ZTST = ztst;
    const GSTEST* t = gsGlobal->Test;
    ps2_gs_write_reg(GS_SETREG_TEST(t->ATE, t->ATST, t->AREF, t->AFAIL,
                                    t->DATE, t->DATM, 1, ztst),
                     GS_TEST_1 + gsGlobal->PrimContext);
    s_gsZtst = ztst;
}

static void ps2_set_zmsk(u8 zmsk) {
    if (!gsGlobal) return;
    if (s_gsZmsk == zmsk) return;
    ps2_gs_write_reg(GS_SETREG_ZBUF(gsGlobal->ZBuffer / 8192, gsGlobal->PSMZ, zmsk),
                     GS_ZBUF_1 + gsGlobal->PrimContext);
    s_gsZmsk = zmsk;
}

// TEXA decides how the single alpha bit of a 16-bit texel expands into the
// 8-bit alpha the blender and the alpha test actually see. That covers every
// texture in this port: they are PSMT8 with a PSMCT16 CLUT, so the CLUT entry's
// one alpha bit goes through TEXA before anything downstream can act on it.
// gsKit does not guarantee a value suited to cutouts and the port never wrote
// the register at all, which leaves a "transparent" texel free to arrive at the
// blender fully opaque — and a region that is RGB 0 / A 0, like a skin's unused
// hat layer, then paints as a solid black box.
//
// TA0 = 0x00 maps A=0 to fully transparent, TA1 = 0x80 maps A=1 to fully opaque
// (0x80 is "1.0" in GS alpha units), and AEM = 1 additionally forces alpha to 0
// for exactly the RGB=0,A=0 texels that make up that hat layer. Texels with A=1
// are untouched, so this can only make genuinely-transparent texels transparent.
static bool s_texaValid = false;

static void ps2_apply_texa() {
    if (!gsGlobal || s_texaValid) return;
    ps2_gs_write_reg(GS_SETREG_TEXA(0x00, 1, 0x80), GS_TEXA);
    s_texaValid = true;
}

static void ps2_clear_depth_only(); // defined after the blend/alpha-test state
static void ps2_apply_color_mask(); // defined with the fixed-function state

// ---- Depth diagnostics ----
//
// The GUI depth path has several independent ways to look broken (test never
// enabled, depth writes masked off, Z pinned to the always-on-top constant, or
// a Z range too narrow to separate two surfaces) and they all present the same
// way on screen: a 3D preview whose parts draw in submission order. These
// counters tell them apart from the console instead of by eye. Zero cost unless
// PS2_RENDER_STATS is defined.
#ifdef PS2_RENDER_STATS
static long s_dbg_depth_clears   = 0; // glClear(kClearDepthBit) this period
static long s_dbg_ortho_z_tested = 0; // ortho verts emitted with the test ON
static long s_dbg_ortho_z_pinned = 0; // ortho verts pinned to PS2_GS_Z_MAX (test OFF)
static int  s_dbg_ortho_z_lo     = 0x7FFFFFFF; // narrowest/widest Z actually emitted
static int  s_dbg_ortho_z_hi     = -1;
// Modelview Z *arriving* at ps2_ortho_gs_z, plus the glOrtho planes in force.
// A collapsed output range (zlo == zhi) has two very different causes -- the
// geometry really is coplanar, or the eye Z is outside [-ofar,-onear] and every
// vertex clamps -- and only the input tells them apart.
static float s_dbg_ortho_oz_lo   = 1e30f;
static float s_dbg_ortho_oz_hi   = -1e30f;
static float s_dbg_ortho_near    = 0.0f;
static float s_dbg_ortho_far     = 0.0f;
static long s_dbg_ortho_sprites   = 0; // GUI quads emitted as GS sprites
static long s_dbg_ortho_sprite_tri = 0; // GUI quads that had to stay triangles
static long s_dbg_zwrite_on      = 0; // draw calls submitted with ZMSK=0 (writing)
static long s_dbg_zwrite_off     = 0; // draw calls submitted with ZMSK=1 (masked)
#define PS2_DEPTH_STAT(expr) do { expr; } while (0)
#else
#define PS2_DEPTH_STAT(expr) do { } while (0)
#endif

// ---- Clear color ----

static u8 s_clearR = 0, s_clearG = 0, s_clearB = 0;

void ps2_render_clear_color(float r, float g, float b, float /*a*/) {
    auto clamp = [](float v) -> u8 {
        if (v <= 0.0f) return 0;
        if (v >= 1.0f) return 255;
        return (u8)(v * 255.0f);
    };
    s_clearR = clamp(r);
    s_clearG = clamp(g);
    s_clearB = clamp(b);
}

void ps2_render_clear(unsigned int mask) {
    if (!gsGlobal) return;
    const bool clearColor = (mask & kClearColorBit) != 0;
    const bool clearDepth = (mask & kClearDepthBit) != 0;
    // Resync the depth register caches at every clear: gsKit_clear re-emits
    // TEST and gsKit's flip path may touch ZBUF behind these caches.
    s_gsZtst = 0xFF;
    s_gsZmsk = 0xFF;
    if (clearColor) {
        // glClear obeys the current color write mask. Program FRAME.FBMSK
        // before gsKit emits its clear sprite. Also mask Z for a color-only
        // clear so the sprite cannot accidentally erase the depth buffer.
        ps2_gs_queue_guard(0);
        ps2_apply_color_mask();
        ps2_set_zmsk(clearDepth ? 0 : 1);

        // gsKit_clear momentarily forces ZTST=ALWAYS and draws a sprite at
        // Z=0. With ZMSK=0 this intentionally clears depth at the same time;
        // with ZMSK=1 it is a true color-only clear.
        gsKit_clear(gsGlobal, GS_SETREG_RGBAQ(s_clearR, s_clearG, s_clearB, 0x80, 0));
        // TEST holds the alpha-test fields (ATE/ATST/AREF/AFAIL) alongside ZTST,
        // and the clear also drives PRIM alpha + the ALPHA register for its own
        // sprite. Dropping only the depth half of the cache left the first draws
        // after a clear believing their alpha test was already programmed, so it
        // was never re-emitted — cutout texels (the transparent hat layer on a
        // player skin, foliage) then drew as opaque black. Invalidate everything
        // the clear can touch; the cost is one redundant register write.
        s_gsZtst = 0xFF;
        s_gsZmsk = 0xFF;
        s_gsFrameMaskValid = false;
        s_alphaTestValid = false;
        s_blendAlphaValid = false;
        s_primAlphaEnableValid = false;
        s_texaValid = false;
    } else if (clearDepth) {
        ps2_clear_depth_only();
    }
    PS2_DEPTH_STAT(if (clearDepth) s_dbg_depth_clears++);
}



// ---- Native fixed-function state ----

struct Ps2BackendState {
    // Vertex arrays
    const void* vp = nullptr;
    int vstride = 0;
    int vsize = 3;
    const void* tp = nullptr;
    int tstride = 0;
    const void* cp = nullptr;
    int cstride = 0;
    int csize = 4;
    bool cfloat = false;

    // Client state
    bool ven = false;
    bool ten = false;
    bool cen = false;

    // Cap state
    bool tex2d = false;
    bool blend = false;
    bool cullFace = false;
    unsigned int cullMode = kFaceBack;
    unsigned int frontFace = kFaceCCW;

    // Flat color
    unsigned char cr = 255;
    unsigned char cg = 255;
    unsigned char cb = 255;
    unsigned char ca = 255;

    // Alpha test
    bool alphaTest = false;
    unsigned int alphaFunc = kCompareGreater;
    u8 alphaRef = 25;

    // Fog
    bool fog = false;
    float fogStart = 0.0f;
    float fogEnd = 32.0f;
    float fogR = 0.7f;
    float fogG = 0.85f;
    float fogB = 1.0f;
    unsigned int fogMode = kFogLinear;
    float fogDensity = 1.0f;
};

static Ps2BackendState st;

// ---- Helpers ----

static inline float gsx(float x) {
    if (!ps2_projection_is_orthographic()) return x;
    return (x - ps2_projection_state().left) / (ps2_projection_state().right - ps2_projection_state().left) * ps2_viewport_width();
}
static inline float gsy(float y) {
    if (!ps2_projection_is_orthographic()) return y;
    // GL Y up → GS Y down
    return (1.0f - (y - ps2_projection_state().bottom) / (ps2_projection_state().top - ps2_projection_state().bottom)) * ps2_viewport_height();
}
static inline u64 mkcol(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
    return GS_SETREG_RGBAQ(r, g, b, (u8)(a >> 1), 0);
}
static inline unsigned char clamp_colorf(float v) {
    if (v <= 0.0f) return 0;
    if (v >= 1.0f) return 255;
    return (unsigned char)(v * 255.0f);
}
static int s_clampMode = -1;
static int s_regionUFix = -1;
static int s_regionVFix = -1;

// Write one clamp selection to the GS. State-cached: a repeat of the current
// selection costs nothing, which is what lets the batching paths ask on every
// primitive. This is the ONLY place that touches gsKit_set_clamp.
static void ps2_apply_clamp_sel(int mode, int ufix, int vfix) {
    if (!gsGlobal || !gsGlobal->Clamp) return;

    switch (mode) {
    case PS2_CLAMPSEL_REPEAT:
        if (s_clampMode != GS_CMODE_REPEAT) {
            gsKit_set_clamp(gsGlobal, GS_CMODE_REPEAT);
            s_clampMode = GS_CMODE_REPEAT;
            s_regionUFix = s_regionVFix = -1;
        }
        return;

    case PS2_CLAMPSEL_REGION:
        if (s_clampMode == GS_CMODE_REGION_REPEAT &&
                s_regionUFix == ufix && s_regionVFix == vfix)
            return;
        gsGlobal->Clamp->MINU = 15;
        gsGlobal->Clamp->MAXU = ufix;
        gsGlobal->Clamp->MINV = 15;
        gsGlobal->Clamp->MAXV = vfix;
        gsKit_set_clamp(gsGlobal, GS_CMODE_REGION_REPEAT);
        s_clampMode = GS_CMODE_REGION_REPEAT;
        s_regionUFix = ufix;
        s_regionVFix = vfix;
        return;

    default:
        if (s_clampMode != GS_CMODE_CLAMP) {
            gsKit_set_clamp(gsGlobal, GS_CMODE_CLAMP);
            s_clampMode = GS_CMODE_CLAMP;
            s_regionUFix = s_regionVFix = -1;
        }
        return;
    }
}

static void ps2_set_texture_clamp_for_uv(GSTEXTURE* tex,
                                         float u0, float v0,
                                         float u1, float v1,
                                         float u2, float v2) {
    if (!gsGlobal || !gsGlobal->Clamp || !tex) return;
    float minU, minV, maxU, maxV;
    ps2_uv_bounds3(u0, v0, u1, v1, u2, v2, minU, minV, maxU, maxV);
    const Ps2ClampSel sel = ps2_select_clamp((float)tex->Width, (float)tex->Height,
                                             ps2_projection_is_orthographic(), ps2_texture_bound_is_tile_atlas(),
                                             minU, minV, maxU, maxV);
    ps2_apply_clamp_sel(sel.mode, sel.ufix, sel.vfix);
}

// Array strides resolved once. The per-index accessors used to redo the
// "stride ? stride : packed" branch and the multiply on every vertex, on a path
// that runs a few hundred thousand times a frame.
struct Ps2ArrayCursor {
    const char* base;
    int stride;
};
static inline Ps2ArrayCursor vp_cursor() {
    Ps2ArrayCursor c;
    c.base = (const char*)st.vp;
    c.stride = st.vstride ? st.vstride : st.vsize * 4;
    return c;
}
static inline Ps2ArrayCursor tp_cursor() {
    Ps2ArrayCursor c;
    c.base = (const char*)st.tp;
    c.stride = st.tstride ? st.tstride : 2 * 4;
    return c;
}
static inline const float* at_(const Ps2ArrayCursor& c, int i) {
    return (const float*)(c.base + i * c.stride);
}

// ---- Fixed-function lighting (entity and GUI models) ----
//
// Minecraft shades every model — mobs, the held item, the 3D item icons and the
// inventory player preview — with RenderHelper::enableStandardItemLighting:
// kCapColorMaterial(AMBIENT_AND_DIFFUSE) plus two directional lights at 0.6 and
// a 0.4 light-model ambient. Without it every face of every box comes out at the
// same brightness, so the boxes stop reading as boxes: the inventory Steve looks
// like a flat blob where the arms melt into the torso and the head into the neck.
// That is entirely a wrapper gap — the game submits the normals (Tessellator
// packs them at byte offset 24 and calls glNormalPointer), the PS2 path just
// threw them away.
//
// Only the diffuse term is evaluated: Minecraft sets both lights' ambient and
// specular to 0, so color = vertexColor * (modelAmbient + Σ diffuse_i·max(0,N·L_i)).
// Cost is bounded to draws that actually enable kCapLighting, i.e. models —
// terrain runs with lighting off and pays one bool test per vertex.
static bool  s_lightingOn = false;
static bool  s_lightOn[2] = { false, false };
// Eye-space light directions: glLightfv(kLightPosition) with w=0 is transformed by
// the modelview in force at that moment, exactly like GL does.
static float s_lightDirEye[2][3] = { {0.0f,1.0f,0.0f}, {0.0f,1.0f,0.0f} };
static float s_lightDiffuse[2][3] = { {1.0f,1.0f,1.0f}, {1.0f,1.0f,1.0f} };
static float s_lightModelAmbient[3] = { 0.2f, 0.2f, 0.2f }; // GL default
static const void* s_np = nullptr;
static int     s_nstride = 0;
static unsigned int      s_ntype = kTypeByte;
static bool        s_nen = false;

static float s_normalMatrix[9] = {
    1.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 1.0f
};
static bool s_lightmapEnabled = false;
static bool s_lightmapColorsValid = false;
static float s_lightmapU = 0.0f;
static float s_lightmapV = 0.0f;
static uint32_t s_lightmapColors[256];

static void ps2_prepare_normal_matrix() {
    const float* m = ps2_matrix_model_view();
    const float a = m[0], b = m[4], c = m[8];
    const float d = m[1], e = m[5], f = m[9];
    const float g = m[2], h = m[6], i = m[10];

    const float c00 = e * i - f * h;
    const float c01 = f * g - d * i;
    const float c02 = d * h - e * g;
    const float c10 = c * h - b * i;
    const float c11 = a * i - c * g;
    const float c12 = b * g - a * h;
    const float c20 = b * f - c * e;
    const float c21 = c * d - a * f;
    const float c22 = a * e - b * d;
    const float determinant = a * c00 + b * c01 + c * c02;

    if (fabsf(determinant) <= 1.0e-12f) {
        s_normalMatrix[0] = a; s_normalMatrix[1] = b; s_normalMatrix[2] = c;
        s_normalMatrix[3] = d; s_normalMatrix[4] = e; s_normalMatrix[5] = f;
        s_normalMatrix[6] = g; s_normalMatrix[7] = h; s_normalMatrix[8] = i;
        return;
    }

    const float inverseDeterminant = 1.0f / determinant;
    s_normalMatrix[0] = c00 * inverseDeterminant;
    s_normalMatrix[1] = c01 * inverseDeterminant;
    s_normalMatrix[2] = c02 * inverseDeterminant;
    s_normalMatrix[3] = c10 * inverseDeterminant;
    s_normalMatrix[4] = c11 * inverseDeterminant;
    s_normalMatrix[5] = c12 * inverseDeterminant;
    s_normalMatrix[6] = c20 * inverseDeterminant;
    s_normalMatrix[7] = c21 * inverseDeterminant;
    s_normalMatrix[8] = c22 * inverseDeterminant;
}

static inline unsigned char ps2_scale_col(unsigned char c, float f) {
    float v = (float)c * f;
    if (v <= 0.0f)   return 0;
    if (v >= 255.0f) return 255;
    return (unsigned char)v;
}

// Modulate one vertex colour by the diffuse lighting at its normal.
static void ps2_light_vertex(int i, unsigned char& r, unsigned char& g, unsigned char& b) {
    if (!s_lightingOn || !s_nen || s_np == nullptr) return;
    if (!s_lightOn[0] && !s_lightOn[1]) return;

    float nx, ny, nz;
    if (s_ntype == kTypeFloat) {
        const int stride = s_nstride ? s_nstride : (int)(3 * sizeof(float));
        const float* n = (const float*)((const char*)s_np + i * stride);
        nx = n[0]; ny = n[1]; nz = n[2];
    } else {
        const int stride = s_nstride ? s_nstride : 3;
        const signed char* n = (const signed char*)s_np + i * stride;
        // Tessellator packs the normal as signed bytes (x*128, y*127, z*127).
        nx = (float)n[0] * (1.0f / 127.0f);
        ny = (float)n[1] * (1.0f / 127.0f);
        nz = (float)n[2] * (1.0f / 127.0f);
    }

    const float ex = s_normalMatrix[0] * nx + s_normalMatrix[1] * ny + s_normalMatrix[2] * nz;
    const float ey = s_normalMatrix[3] * nx + s_normalMatrix[4] * ny + s_normalMatrix[5] * nz;
    const float ez = s_normalMatrix[6] * nx + s_normalMatrix[7] * ny + s_normalMatrix[8] * nz;
    const float len2 = ex*ex + ey*ey + ez*ez;
    if (len2 < 1e-12f) return;
    const float inv = 1.0f / sqrtf(len2);

    float fr = s_lightModelAmbient[0];
    float fg = s_lightModelAmbient[1];
    float fb = s_lightModelAmbient[2];
    for (int l = 0; l < 2; l++) {
        if (!s_lightOn[l]) continue;
        float d = (ex * s_lightDirEye[l][0] +
                   ey * s_lightDirEye[l][1] +
                   ez * s_lightDirEye[l][2]) * inv;
        if (d <= 0.0f) continue;
        fr += d * s_lightDiffuse[l][0];
        fg += d * s_lightDiffuse[l][1];
        fb += d * s_lightDiffuse[l][2];
    }
    // GL clamps the lit colour to [0,1] per channel.
    if (fr > 1.0f) fr = 1.0f;
    if (fg > 1.0f) fg = 1.0f;
    if (fb > 1.0f) fb = 1.0f;

    r = ps2_scale_col(r, fr);
    g = ps2_scale_col(g, fg);
    b = ps2_scale_col(b, fb);
}

static void ps2_apply_lightmap(unsigned char& r, unsigned char& g, unsigned char& b) {
    if (!s_lightmapEnabled || !s_lightmapColorsValid)
        return;

    int blockLight = (int)s_lightmapU >> 4;
    int skyLight = (int)s_lightmapV >> 4;
    if (blockLight < 0) blockLight = 0; else if (blockLight > 15) blockLight = 15;
    if (skyLight < 0) skyLight = 0; else if (skyLight > 15) skyLight = 15;

    const uint32_t light = s_lightmapColors[skyLight * 16 + blockLight];
    const unsigned int lightR = (light >> 16) & 0xffu;
    const unsigned int lightG = (light >> 8) & 0xffu;
    const unsigned int lightB = light & 0xffu;
    r = (unsigned char)(((unsigned int)r * lightR + 127u) / 255u);
    g = (unsigned char)(((unsigned int)g * lightG + 127u) / 255u);
    b = (unsigned char)(((unsigned int)b * lightB + 127u) / 255u);
}

static void getcol(int i, unsigned char& r, unsigned char& g, unsigned char& b, unsigned char& a) {
    if (!st.cen || !st.cp) {
        r=st.cr; g=st.cg; b=st.cb; a=st.ca;
        ps2_light_vertex(i, r, g, b);
        ps2_apply_lightmap(r, g, b);
        return;
    }
    if (st.cfloat) {
        int s2 = st.cstride ? st.cstride : st.csize * 4;
        const float* c = (const float*)((const char*)st.cp + i * s2);
        r=clamp_colorf(c[0]); g=clamp_colorf(c[1]); b=clamp_colorf(c[2]);
        a = (st.csize>=4) ? clamp_colorf(c[3]) : 255;
    } else {
        int s2 = st.cstride ? st.cstride : st.csize;
        const unsigned char* c = (const unsigned char*)st.cp + i * s2;
        r=c[0]; g=c[1]; b=c[2]; a=(st.csize>=4)?c[3]:255;
    }
    ps2_light_vertex(i, r, g, b);
    ps2_apply_lightmap(r, g, b);
}

// ---- Native fixed-function implementation ----

// Stored blend factors, updated by RenderAPI.
static unsigned int s_blendSrc = kBlendSrcAlpha;
static unsigned int s_blendDst = kBlendOneMinusSrcAlpha;
static bool s_primAlphaEnable = false;
static u64 s_blendAlphaReg = 0;
static unsigned int s_blendEquationRGB = kBlendEquationAdd;

// Extra fixed-function state. Several desktop GL calls used by Minecraft were
// previously inline no-ops in gles_ps2.h. Keeping them here prevents state leaks
// between world, GUI, inventory and transparent overlay draws.
static bool s_depthTestEnabled = true;
static bool s_depthMaskEnabled = true;
static unsigned int s_depthFunc = kCompareLessEqual;
static unsigned int s_depthFuncBeforeTranslucent = kCompareLessEqual;
static bool s_polygonOffsetFill = false;
static float s_polygonOffsetUnits = 0.0f;

// The GS has no slope-scaled depth-bias register. Minecraft 1.2.5 uses
// PolygonOffsetFill for the block-damage overlay with both factor and units
// set to -3. The constant units term is enough to separate those coplanar
// faces. PS2 depth is reversed (larger is nearer), so negative GL units must
// become a positive GS depth offset. Keep the bias deliberately small so the
// overlay does not jump in front of neighboring block faces.
static inline float ps2_polygon_depth_bias_gs() {
    if (!s_polygonOffsetFill || s_polygonOffsetUnits == 0.0f)
        return 0.0f;
    return -s_polygonOffsetUnits * 2.0f;
}

static inline int ps2_apply_polygon_depth_bias(int depth) {
    const float bias = ps2_polygon_depth_bias_gs();
    if (bias == 0.0f)
        return depth;
    int adjusted = depth + static_cast<int>(bias);
    if (adjusted < 0) adjusted = 0;
    if (adjusted > PS2_GS_Z_MAX) adjusted = PS2_GS_Z_MAX;
    return adjusted;
}
static bool s_translucentDepthFuncOverridden = false;
static bool s_colorMaskR = true, s_colorMaskG = true, s_colorMaskB = true, s_colorMaskA = true;
static unsigned int s_shadeModel = kShadeSmooth;

static u32 ps2_color_write_mask()
{
    if (!gsGlobal)
        return 0;

    u32 mask = 0;
    if (gsGlobal->PSM == GS_PSM_CT16 || gsGlobal->PSM == GS_PSM_CT16S)
    {
        // FRAME.FBMSK is applied to the 32-bit RGBA value BEFORE the GS
        // converts it to PSMCT16.  Therefore the writable RGB5A1 bits map to
        // source bits 3..7, 11..15, 19..23 and 31 respectively.  Treating
        // FBMSK as two packed native 16-bit pixels (the old implementation)
        // masks unrelated source bits and can leave one gsKit context unable
        // to update its framebuffer.  This is the same mapping used by
        // PS2SDK's GS_SET_FMSK16 helper.
        if (!s_colorMaskR) mask |= 0x000000F8u;
        if (!s_colorMaskG) mask |= 0x0000F800u;
        if (!s_colorMaskB) mask |= 0x00F80000u;
        if (!s_colorMaskA) mask |= 0x80000000u;
        return mask;
    }

    if (!s_colorMaskR) mask |= 0x000000FFu;
    if (!s_colorMaskG) mask |= 0x0000FF00u;
    if (!s_colorMaskB) mask |= 0x00FF0000u;
    if (!s_colorMaskA) mask |= 0xFF000000u;
    return mask;
}

static void ps2_apply_color_mask()
{
    if (!gsGlobal)
        return;

    // gsKit_sync_flip() moves the draw target to the other ScreenBuffer without
    // touching PrimContext, so FRAME.FBP changes from frame to frame while the
    // colour mask usually does not. Both halves of the register therefore have
    // to be part of the cache key: keying on the mask alone made a stale hit
    // suppress the write that follows a buffer swap.
    const u32 mask = ps2_color_write_mask();
    const u32 drawBuffer = gsGlobal->ScreenBuffer[gsGlobal->ActiveBuffer & 1];
    const u32 fbp = drawBuffer / 8192;
    if (s_gsFrameMaskValid && s_gsFrameMask == mask && s_gsFrameFbp == fbp)
        return;

    // gsKit_setactive() points BOTH GS contexts at the current draw buffer.
    // Do the same when restoring FBMSK. Mapping FRAME_1 to ScreenBuffer[0] and
    // FRAME_2 to ScreenBuffer[1] is incorrect for gsKit double buffering: on
    // alternating frames it redirects drawing back into the displayed buffer,
    // leaving the intended backbuffer stale and producing an every-other-frame
    // flicker as soon as anything on screen animates.
    const u64 frame = GS_SETREG_FRAME(fbp, gsGlobal->Width / 64,
                                      gsGlobal->PSM, mask);
    ps2_gs_write_reg(frame, GS_FRAME_1);
    ps2_gs_write_reg(frame, GS_FRAME_2);

    s_gsFrameMaskValid = true;
    s_gsFrameMask = mask;
    s_gsFrameFbp = fbp;
}

static void ps2_set_blend_alpha(u64 alphaReg) {
    if (!gsGlobal) return;
    if (s_blendAlphaValid && s_blendAlphaReg == alphaReg)
        return;
    gsKit_set_primalpha(gsGlobal, alphaReg, 0);
    s_blendAlphaValid = true;
    s_blendAlphaReg = alphaReg;
}

// World transparent pass (water/ice): CT16/PSMT8 textures carry only 1-bit
// alpha, so the standard As-driven blend collapses to fully opaque there.
// While forced, blend with a constant factor instead: (Cs-Cd)*FIX/128+Cd.
static bool s_blendFixForced = false;
static u8 s_blendFixValue = 0x80;

// Apply the current blend equation to gsGlobal
static void ps2_apply_blend() {
    if (!gsGlobal) return;
    // GS ALPHA register: (A-B)*C/128 + D
    // A=0=Cs A=1=Cd A=2=0
    // B=0=Cs B=1=Cd B=2=0
    // C=0=As C=1=Ad C=2=FIX
    // D=0=Cs D=1=Cd D=2=0
    if (s_blendFixForced) {
        ps2_set_blend_alpha(GS_SETREG_ALPHA(0, 1, 2, 1, s_blendFixValue));
        return;
    }
    u64 alphaReg;

    // Minecraft normally uses FUNC_ADD. Other equations are approximated so
    // code using the desktop API still has deterministic output on PS2.
    if (s_blendEquationRGB == kBlendEquationReverseSubtract) {
        // Cd - Cs, approximated at full strength.
        alphaReg = GS_SETREG_ALPHA(1,0,2,2,0x80);
    } else if (s_blendEquationRGB == kBlendEquationSubtract) {
        // Cs - Cd, approximated at full strength.
        alphaReg = GS_SETREG_ALPHA(0,1,2,2,0x80);
    } else if (s_blendSrc == kBlendSrcAlpha && s_blendDst == kBlendOneMinusSrcAlpha) {
        // Standard alpha: (Cs-Cd)*As/128+Cd
        alphaReg = GS_SETREG_ALPHA(0,1,0,1,0);
    } else if (s_blendSrc == kBlendOne && s_blendDst == kBlendOneMinusSrcAlpha) {
        // Pre-multiplied alpha: use the standard equation as closest match.
        alphaReg = GS_SETREG_ALPHA(0,1,0,1,0);
    } else if (s_blendSrc == kBlendSrcAlpha && s_blendDst == kBlendOne) {
        // Additive particle/lightning style: (Cs-0)*As+Cd
        alphaReg = GS_SETREG_ALPHA(0,2,0,1,0);
    } else if (s_blendSrc == kBlendOne && s_blendDst == kBlendOne) {
        // Additive: Cs+Cd = (Cs-0)*1+Cd
        alphaReg = GS_SETREG_ALPHA(0,2,2,1,0x80);
    } else if (s_blendSrc == kBlendDstColor && s_blendDst == kBlendSrcColor) {
        // Multiplicative overlays are not directly representable; keep a sane
        // darkening-ish approximation instead of leaving stale blend state.
        alphaReg = GS_SETREG_ALPHA(1,0,2,2,0x40);
    } else if (s_blendSrc == kBlendZero && s_blendDst == kBlendOneMinusSrcColor) {
        // Vignette/pumpkin overlay approximation.
        alphaReg = GS_SETREG_ALPHA(1,0,2,2,0x80);
    } else if (s_blendSrc == kBlendOneMinusDstColor && s_blendDst == kBlendOneMinusSrcColor) {
        // Crosshair/cursor invert: approximate with standard alpha.
        alphaReg = GS_SETREG_ALPHA(0,1,0,1,0);
    } else {
        // Fallback: standard alpha blend.
        alphaReg = GS_SETREG_ALPHA(0,1,0,1,0);
    }
    ps2_set_blend_alpha(alphaReg);
}

// Compatibility hook for callers that still need to override the GS blend
// factor explicitly. Native terrain pass setup owns this state directly now.
// fix is in GS units (0x80 = 100% source).
extern "C" void ps2_force_fix_blend(int enable, unsigned char fix) {
    s_blendFixForced = enable != 0;
    s_blendFixValue = fix;
    s_blendAlphaValid = false; // drop the cache so the next apply re-emits
    if (st.blend)
        ps2_apply_blend();
}

void ps2_native_begin_terrain_pass(unsigned int textureId, Ps2NativeTerrainPass pass) {
    st.tex2d = true;
    ps2_texture_bind((unsigned int)textureId);

    if (pass == PS2_NATIVE_TERRAIN_TRANSLUCENT) {
        // The PS2 pass intentionally uses fixed-factor blending because the
        // PSMT8/CT16 terrain atlas only carries one alpha bit. Configure the
        // exact state the old GL call sequence produced, but do it once here.
        s_blendSrc = kBlendSrcAlpha;
        s_blendDst = kBlendOneMinusSrcAlpha;
        st.blend = true;
        st.cullFace = false;
        s_depthMaskEnabled = false;

        // PSMZ_16S only has 15 usable depth bits. At medium distance an opaque
        // grass face and water one level below it can quantise to the same Z.
        // The normal LEQUAL mapping becomes GS GEQUAL, so that tie lets the
        // later translucent pass paint water through the ground. Use strict
        // LESS/GREATER for pass 1: genuinely visible water still beats the
        // depth buffer, while a quantised tie against opaque terrain does not.
        // Keep the override local because entities, the hand and GUI retain
        // the compatibility layer's normal LEQUAL semantics.
        if (!s_translucentDepthFuncOverridden) {
            s_depthFuncBeforeTranslucent = s_depthFunc;
            s_translucentDepthFuncOverridden = true;
        }
        s_depthFunc = kCompareLess;

        s_blendFixForced = true;
        s_blendFixValue = 0x58;
        s_blendAlphaValid = false;
        ps2_apply_blend();
        return;
    }

    // Opaque terrain is the baseline world state. Explicitly restore it so a
    // previous transparent/overlay draw cannot leak blend or depth-write state
    // into the next terrain pass. Alpha-test is left untouched: cutout blocks
    // still use the existing per-draw GS test setup.
    st.blend = false;
    st.cullFace = true;
    s_depthMaskEnabled = true;
    s_blendFixForced = false;
    s_blendFixValue = 0x80;
}

void ps2_native_end_terrain_pass(Ps2NativeTerrainPass pass) {
    if (pass != PS2_NATIVE_TERRAIN_TRANSLUCENT)
        return;

    // Match the vanilla/legacy post-pass baseline used by entity, hand and HUD
    // rendering. The next draw re-applies its cached GS registers as needed.
    s_blendFixForced = false;
    s_blendFixValue = 0x80;
    s_blendAlphaValid = false;
    s_depthMaskEnabled = true;
    if (s_translucentDepthFuncOverridden) {
        s_depthFunc = s_depthFuncBeforeTranslucent;
        s_translucentDepthFuncOverridden = false;
    }
    st.cullFace = true;
    st.blend = false;
}

void ps2_render_blend_func(unsigned int source, unsigned int destination) {
    s_blendSrc = (unsigned int)source;
    s_blendDst = (unsigned int)destination;
    if (st.blend)
        ps2_apply_blend();
}





// ---- Alpha test (GS TEST register) ----
// Cached so we only emit a TEST GIF packet when the state actually changes.
// s_alphaTestValid is declared with the other GS state caches near the top.
static bool s_alphaTestOn    = false;
static u8   s_alphaTestAtst  = 0;
static u8   s_alphaTestAref  = 0;

// Map a GL comparison func to the GS ATST field.
static u8 ps2_compare_to_atst(unsigned int f) {
    switch (f) {
        case kCompareNever:    return 0; // NEVER
        case kCompareLess:     return 2; // LESS
        case kCompareLessEqual:   return 3; // LEQUAL
        case kCompareEqual:    return 4; // EQUAL
        case kCompareGreaterEqual:   return 5; // GEQUAL
        case kCompareGreater:  return 6; // GREATER
        case kCompareNotEqual: return 7; // NOTEQUAL
        case kCompareAlways:
        default:          return 1; // ALWAYS
    }
}

// Push the current alpha-test state into the GS. Must run inside the draw
// frame (called from the native array renderer), where gsKit_set_test queues the packet.
static void ps2_apply_alpha_test() {
    if (!gsGlobal || !gsGlobal->Test) return;

    u8 atst = ps2_compare_to_atst(st.alphaFunc);
    u8 aref = st.alphaRef;

    if (s_alphaTestValid && s_alphaTestOn == st.alphaTest
            && s_alphaTestAtst == atst && s_alphaTestAref == aref)
        return;

    if (st.alphaTest) {
        gsGlobal->Test->ATST  = atst;
        gsGlobal->Test->AREF  = aref;
        gsGlobal->Test->AFAIL = 0; // KEEP: failed texels write nothing (clean cutout)
        gsKit_set_test(gsGlobal, GS_ATEST_ON);
    } else {
        gsKit_set_test(gsGlobal, GS_ATEST_OFF);
    }

    s_alphaTestValid = true;
    s_alphaTestOn    = st.alphaTest;
    s_alphaTestAtst  = atst;
    s_alphaTestAref  = aref;
}

void ps2_render_alpha_func(unsigned int compare, float reference) {
    st.alphaFunc = (unsigned int)compare;
    if (reference <= 0.0f)      st.alphaRef = 0;
    else if (reference >= 1.0f) st.alphaRef = 255;
    else                        st.alphaRef = (u8)(reference * 255.0f);
}


// Real glClear(kClearDepthBit): draw a fullscreen untextured sprite that
// leaves the color buffer untouched — blend (Cs-Cd)*FIX(0)/128 + Cd = Cd —
// while writing Z=0 (the farthest value in the reversed near=0xFFFF mapping)
// to every pixel. Minecraft relies on mid-frame depth clears for the
// item-in-hand (never clips into terrain) and every GUI overlay/preview.
static void ps2_clear_depth_only() {
    if (!gsGlobal || !gsGlobal->Test) return;

    // Reserve before writing. gsKit_set_test / gsKit_set_primalpha /
    // gsKit_prim_sprite all push into Os_Queue and gsKit never bounds-checks
    // (the check in _gsKit_heap_alloc is behind #ifdef GSKIT_DEBUG), so a depth
    // clear issued on an already-full queue writes past pool_max into the C++
    // heap. Every other draw site in this file is guarded; this one was not,
    // and it is on the hottest possible path for it — every GUI screen that
    // shows a 3D preview (inventory, crafting, furnace, chest) calls
    // glClear(kClearDepthBit) once per frame, after the world and HUD have
    // already filled the queue. The payload is a few registers plus one sprite,
    // far under the guard's fixed 8KB slack, so 0 vertices is the right ask.
    ps2_gs_queue_guard(0);

    ps2_set_zmsk(0); // write Z
    ps2_set_ztst(1); // ALWAYS: win regardless of the old depth contents

    // The sprite must not be discarded by an inherited alpha test.
    gsKit_set_test(gsGlobal, GS_ATEST_OFF);
    s_alphaTestValid = false;

    // Keep-destination blend; the next draw re-applies its own blend state.
    gsGlobal->PrimAlphaEnable = GS_SETTING_ON;
    s_primAlphaEnableValid = false;
    gsKit_set_primalpha(gsGlobal, GS_SETREG_ALPHA(0, 1, 2, 1, 0x00), 0);
    s_blendAlphaValid = false;

    gsKit_prim_sprite(gsGlobal, 0.0f, 0.0f,
                      (float)gsGlobal->Width, (float)gsGlobal->Height,
                      0, GS_SETREG_RGBAQ(0, 0, 0, 0x80, 0));
}

// Push the GL depth state into the GS for the next draw call.
// Reversed depth mapping (near = 0x7FFF, far = 0) flips LESS/LEQUAL into the
// GS-native GREATER/GEQUAL tests. The GS cannot express the reverse direction
// (LESS/LEQUAL), EQUAL or NOTEQUAL, so those uncommon GL modes use documented
// conservative fallbacks instead of silently applying the opposite comparison.
// GL semantics: depth test disabled = always pass AND never write.
static void ps2_apply_depth_state() {
    u8 ztst, zmsk;
    if (s_depthTestEnabled) {
        static unsigned int warnedUnsupported = 0;
        switch (s_depthFunc) {
            case kCompareAlways: ztst = 1; break;
            case kCompareNever:  ztst = 0; break;
            case kCompareLess:   ztst = 3; break; // reversed depth -> GS GREATER
            case kCompareLessEqual: ztst = 2; break; // reversed depth -> GS GEQUAL
            case kCompareEqual:
                // The GS has no EQUAL Z test. GEQUAL preserves the required
                // equality and is the least destructive fallback for the
                // RenderLiving overlay passes that use it.
                ztst = 2;
                if ((warnedUnsupported & 1u) == 0) {
                    warnedUnsupported |= 1u;
                    MC_LOG_WARN("render", "[PS2] depth EQUAL approximated with GEQUAL\n");
                }
                break;
            case kCompareGreater:
            case kCompareGreaterEqual:
            case kCompareNotEqual:
                // Reversed GREATER/GEQUAL would require GS LESS/LEQUAL, which
                // the hardware cannot express. ALWAYS avoids applying the
                // opposite comparison; Minecraft's only GEQUAL use is the
                // achievement-map background, which is overdrawn afterwards.
                ztst = 1;
                if ((warnedUnsupported & 2u) == 0) {
                    warnedUnsupported |= 2u;
                    MC_LOG_WARN("render", "[PS2] unsupported reverse depth compare approximated with ALWAYS\n");
                }
                break;
            default: ztst = 2; break;
        }
        zmsk = s_depthMaskEnabled ? 0 : 1;
    } else {
        ztst = 1;
        zmsk = 1;
    }
    ps2_set_ztst(ztst);
    ps2_set_zmsk(zmsk);
    PS2_DEPTH_STAT(zmsk == 0 ? s_dbg_zwrite_on++ : s_dbg_zwrite_off++);
}

#define PS2_kCapFog        0x0B60
#define PS2_kFogStart  0x0B63
#define PS2_kFogEnd    0x0B64
#define PS2_kFogColor  0x0B66

void ps2_render_enable(unsigned int cap) {
    if (cap == kCapTexture2D) st.tex2d = true;
    else if (cap == kCapBlend) {
        st.blend = true;
        ps2_apply_blend();
    }
    else if (cap == kCapAlphaTest) st.alphaTest = true;
    else if (cap == kCapCullFace) st.cullFace = true;
    else if (cap == kCapDepthTest) s_depthTestEnabled = true;
    else if (cap == PS2_kCapFog) st.fog = true;
    else if (cap == kCapLighting) s_lightingOn = true;
    else if (cap == kCapLight0) s_lightOn[0] = true;
    else if (cap == kCapLight1) s_lightOn[1] = true;
    else if (cap == kCapPolygonOffsetFill) s_polygonOffsetFill = true;
    else if (cap == kCapNormalize || cap == kCapRescaleNormal) {
        // ps2_light_vertex normalizes the eye-space normal unconditionally, so
        // NORMALIZE/RESCALE_NORMAL need no separate state.
    }
}

void ps2_render_disable(unsigned int cap) {
    if (cap == kCapTexture2D) st.tex2d = false;
    else if (cap == kCapBlend) st.blend = false;
    else if (cap == kCapAlphaTest) st.alphaTest = false;
    else if (cap == kCapCullFace) st.cullFace = false;
    else if (cap == kCapDepthTest) s_depthTestEnabled = false;
    else if (cap == PS2_kCapFog) st.fog = false;
    else if (cap == kCapLighting) s_lightingOn = false;
    else if (cap == kCapLight0) s_lightOn[0] = false;
    else if (cap == kCapLight1) s_lightOn[1] = false;
    else if (cap == kCapPolygonOffsetFill) s_polygonOffsetFill = false;
    else if (cap == kCapNormalize || cap == kCapRescaleNormal) {
        // No-op, but accepted.
    }
}

void ps2_render_fogf(unsigned int parameter, float value) {
    if (parameter == kFogStart) st.fogStart = value;
    else if (parameter == kFogEnd) st.fogEnd = value;
    else if (parameter == ps2RenderValue(Ps2RenderFogParameter::Density)) st.fogDensity = value;
}

void ps2_render_fog_color(const float* values) {
    if (!values) return;
    st.fogR = values[0]; st.fogG = values[1]; st.fogB = values[2];
}

void ps2_render_fog_mode(unsigned int mode) {
    if (mode == kFogLinear || mode == kFogExp || mode == kFogExp2)
        st.fogMode = mode;
}


void ps2_render_cull_face(unsigned int face) {
    if (face == kFaceFront || face == kFaceBack) st.cullMode = (unsigned int)face;
}


void ps2_render_depth_func(unsigned int compare) {
    s_depthFunc = (unsigned int)compare;
    // Applied per draw call by ps2_apply_depth_state() (TEST.ZTST, with the
    // compare direction flipped for the reversed near=0xFFFF GS mapping).
}

void ps2_render_depth_mask(bool enabled) {
    s_depthMaskEnabled = enabled;
    // Applied per draw call by ps2_apply_depth_state() (ZBUF.ZMSK).
}

void ps2_render_color_mask(bool red, bool green, bool blue, bool alpha) {
    s_colorMaskR = red;
    s_colorMaskG = green;
    s_colorMaskB = blue;
    s_colorMaskA = alpha;
}

void ps2_render_shade_model(unsigned int model) {
    if (model == kShadeFlat || model == kShadeSmooth)
        s_shadeModel = (unsigned int)model;
}

void ps2_render_viewport(int x, int y, int width, int height) {
    ps2_viewport_set(x, y, width, height);
}

void ps2_render_get_viewport(int* values) {
    ps2_viewport_get(values);
}

// Directional lights only: Minecraft never uses positional lights, and the w
// component it passes is always 0. The direction is taken into eye space with
// the modelview that is current right now, which is what GL does at the moment
// glLight is called — GuiContainer relies on it (it sets the item lights inside
// a 120-degree X rotation) and so does the world entity pass (camera matrix).
void ps2_render_lightfv(int lightIndex, unsigned int parameter, const float* params) {
    const int idx = (lightIndex == 0 || lightIndex == 1) ? lightIndex : -1;
    const unsigned int pname = (unsigned int)parameter;
    if (idx < 0 || params == nullptr) return;
    if (pname == kLightPosition) {
		const float* m = ps2_matrix_model_view();
        const float x = params[0], y = params[1], z = params[2];
        float ex = m[0]*x + m[4]*y + m[8]*z;
        float ey = m[1]*x + m[5]*y + m[9]*z;
        float ez = m[2]*x + m[6]*y + m[10]*z;
        const float len2 = ex*ex + ey*ey + ez*ez;
        if (len2 < 1e-12f) return;
        const float inv = 1.0f / sqrtf(len2);
        s_lightDirEye[idx][0] = ex * inv;
        s_lightDirEye[idx][1] = ey * inv;
        s_lightDirEye[idx][2] = ez * inv;
    } else if (pname == kLightDiffuse) {
        s_lightDiffuse[idx][0] = params[0];
        s_lightDiffuse[idx][1] = params[1];
        s_lightDiffuse[idx][2] = params[2];
    }
    // kLightAmbient / kLightSpecular are set to 0 by RenderHelper; nothing to do.
}


void ps2_render_light_model_ambient(const float* values) {
    if (values == nullptr) return;
    s_lightModelAmbient[0] = values[0];
    s_lightModelAmbient[1] = values[1];
    s_lightModelAmbient[2] = values[2];
}

// The PS2 lighting path treats the vertex colour as the material, which is
// exactly what kAmbientAndDiffuse means and the only mode Minecraft ever
// selects. There is no separate material to track, so this records nothing.
void ps2_render_color_material(unsigned int face, unsigned int mode) {
    (void)face;
    (void)mode;
}

void ps2_render_set_lightmap_enabled(bool enabled) {
    s_lightmapEnabled = enabled;
}

void ps2_render_set_lightmap_coord(float u, float v) {
    s_lightmapU = u;
    s_lightmapV = v;
}

void ps2_render_set_lightmap_colors(const unsigned int* colors, int count) {
    if (colors == nullptr || count < 256) {
        s_lightmapColorsValid = false;
        return;
    }
    memcpy(s_lightmapColors, colors, sizeof(s_lightmapColors));
    s_lightmapColorsValid = true;
}


void ps2_render_color4f(float r, float g, float b, float a) {
    st.cr=clamp_colorf(r); st.cg=clamp_colorf(g);
    st.cb=clamp_colorf(b); st.ca=clamp_colorf(a);
}

// The GS interpolates the vertex normal only through the lighting path, which
// reads its normals from the client array. A single current normal has no
// consumer here, so this is accepted and dropped -- the same contract the
// glNormal3f stub had.
void ps2_render_normal3f(float x, float y, float z) {
    (void)x;
    (void)y;
    (void)z;
}

// Owns what the glFrustum stub in gles_ps2.h used to build inline. The GS depth
// mapping needs the near/far planes and they cannot be recovered from clip
// space, so they are recorded here before the matrix is multiplied in.
static void ps2_render_set_frustum_planes(float zNear, float zFar);
void ps2_render_frustum(float left, float right, float bottom, float top,
                        float nearValue, float farValue) {
    ps2_render_set_frustum_planes(nearValue, farValue);
    Mat4 m;
    memset(m, 0, sizeof(m));
    // Each extent divides two entries, so it is reciprocated once. That is a
    // rounding step the plain divisions did not have, which is far below what a
    // projection matrix can resolve, and it halves the divide count.
    const float invWidth  = 1.0f / (right - left);
    const float invHeight = 1.0f / (top - bottom);
    const float invDepth  = 1.0f / (farValue - nearValue);
    m[0]  = 2.0f * nearValue * invWidth;
    m[5]  = 2.0f * nearValue * invHeight;
    m[8]  = (right + left) * invWidth;
    m[9]  = (top + bottom) * invHeight;
    m[10] = -(farValue + nearValue) * invDepth;
    m[11] = -1.0f;
    m[14] = -2.0f * farValue * nearValue * invDepth;
	ps2_matrix_multiply_current(m);
}

void ps2_render_ortho(float left, float right, float bottom, float top,
                      float nearValue, float farValue) {
    ps2_projection_set_ortho(left, right, bottom, top, nearValue, farValue);
}


// ---- Matrix stack functions ----

void ps2_render_matrix_mode(unsigned int mode) {
    ps2_matrix_set_mode(mode);
}

void ps2_render_load_identity() {
    if (ps2_matrix_load_identity())
        ps2_projection_clear_ortho();
}

void ps2_render_push_matrix() {
    ps2_projection_push(ps2_matrix_current_mode());
    ps2_matrix_push();
}

void ps2_render_pop_matrix() {
    ps2_projection_pop(ps2_matrix_current_mode());
    ps2_matrix_pop();
}

void ps2_render_get_matrix(unsigned int query, float* values) {
    ps2_matrix_get(query, values);
}


void ps2_render_translate(float x, float y, float z) {
	ps2_matrix_translate(x, y, z);
}
void ps2_render_scale(float x, float y, float z) {
	ps2_matrix_scale(x, y, z);
}
void ps2_render_rotate(float angle, float ax, float ay, float az) {
	ps2_matrix_rotate(angle, ax, ay, az);
}


// The remaining native entry points have no state of their own on this backend.
// They are real functions rather than header stubs so the reason each one does
// nothing is recorded once, next to the pipeline it would have driven.
//
//   clear_depth      ps2_clear_depth_only() writes the far value unconditionally;
//                    the GS has no clear-value register to preload.
//   polygon_offset   the GS rasterises with no depth bias unit. Minecraft uses
//                    this only for the block-breaking overlay, which the PS2
//                    path already separates with its own draw order.
//   line_width       GS lines are one pixel wide; there is no width register.
//   fog_hint         the fog blend is per-vertex linear either way.
void ps2_render_clear_depth(float depth) {
    (void)depth;
}
void ps2_render_polygon_offset(float factor, float units) {
    // The GS has no slope-scaled polygon offset, so only the constant units
    // term can be reproduced by this backend.
    (void)factor;
    s_polygonOffsetUnits = units;
}
void ps2_render_line_width(float width) {
    (void)width;
}
void ps2_render_fog_hint(bool nicest) {
    (void)nicest;
}

const unsigned char* ps2_render_get_string() {
    return (const unsigned char*)"PS2";
}

// The backend reports its failures through the unified PS2 log and by
// invalidating the object that failed (see ps2_texture_upload_rgba), not
// through a deferred error queue nothing reads.
unsigned int ps2_render_get_error() {
    return PS2_RENDER_NO_ERROR;
}





// The GL wrap/filter enums do not survive the trip to the GS one-to-one, so
// the compatibility layer collapses each pname to the boolean the native API
// takes and lets Ps2Texture document what the hardware does with it.



static bool is_culled_3d(float x0, float y0, float x1, float y1, float x2, float y2) {
    if (!st.cullFace || ps2_projection_is_orthographic()) return false;

    const float area = (x1 - x0) * (y2 - y0) - (y1 - y0) * (x2 - x0);
    if (fabsf(area) < 0.0001f) return false;

    // GS screen space is Y-down, so GL's default CCW front face appears CW here.
    const bool front = (st.frontFace == kFaceCCW) ? (area < 0.0f) : (area > 0.0f);
    return (st.cullMode == kFaceBack) ? !front : front;
}

#ifdef PS2_RENDER_STATS
static long s_dbg_3d_prim = 0;     // 3D prims with all verts in front of camera
// Rejects are split because they cost very different amounts and have very
// different fixes. `clip` is the cheap one: the outcodes agreed the triangle is
// wholly beyond one frustum plane, so it dies after the VU0 transform and
// nothing else. `offscreen` is the expensive one: the triangle passed the
// frustum test, paid a perspective divide per vertex, and only then failed the
// screen-space box test -- which means the geometry was submitted by a section
// the per-section frustum cull already accepted. A high `offscreen` points at
// the section granularity (the renderer AABB carries a 6-block margin), a high
// `clip` just means the view is small relative to the loaded grid.
static long s_dbg_3d_clip = 0;      // dropped by outcode/near-plane clip
static long s_dbg_3d_offscreen = 0; // dropped by the post-projection screen box
static long s_dbg_3d_backface = 0;  // dropped by backface culling
static long s_dbg_3d_calls = 0;    // 3D draws reaching the renderer
static long s_dbg_native_attempts = 0;
static long s_dbg_native_hits = 0;
// Queue sizing evidence. PS2_GSKIT_OS_QUEUE_BYTES costs twice its value in RAM
// (pool[2]) and is the single largest allocation in the game, but it must not be
// shrunk on reasoning about the guard — only on a measured worst-case frame.
// These give that measurement:
//   qpeak  = most bytes ever live in one pool fill. The pool only has to hold
//            this much (plus slack) to keep the flush rate where it is today.
//   qdraw  = largest packet a single guarded draw actually wrote.
//   qover  = times a draw wrote MORE than its guard reserved. Must stay 0; any
//            other value means the estimate is unsafe and the current margin is
//            the only thing preventing a write past pool_max into the C++ heap.
#define PS2_RENDER_STAT(expr) do { expr; } while (0)
#else
#define PS2_RENDER_STAT(expr) do { } while (0)
#endif

// ClipVert / lerp_cv / outcodes / ps2_clip_poly_guard live in Ps2ClipGuard.h
// (shared with the fast-draw paths).

// Triangles staged before one textured GUI/HUD list is handed to gsKit.
// 32 tris = 96 vertices = 4.5KB of BSS, and collapses the GIF tag + TEX0 pair
// that used to precede every single triangle down to one per batch. Larger
// batches buy little: GUI draw calls are short (a text string is a few dozen
// quads) and the array is permanently resident on a 32MB console.
#define PS2_ORTHO_BATCH_TRIS 32

// When on, every projected vertex is pinned to the near-most GS Z (PS2_GS_Z_MAX) so
// the geometry always draws on top of the world. Used for the item-in-hand:
// the GS can't clear the depth buffer mid-frame, so without this the hand
// z-clips into nearby terrain. GameRenderer brackets renderItemInHand() with
// ps2glForceNearZ(true/false).
//
// Both emitters honour it: the generic path in project_cv below, the fast path
// through Ps2Draw3DState::forceNearZ. It used to be a fast-path
// disqualifier instead, which quietly sent the held item -- geometry drawn on
// every single frame -- down the per-triangle generic emitter.
static bool s_forceNearZ = false;
void ps2glForceNearZ(bool on) { s_forceNearZ = on; }

// GS depth for an ortho (GUI) vertex from its modelview-transformed Z.
// With glOrtho(n,f) GL maps eye-space z as z_ndc = (-2*z - (f+n)) / (f-n);
// the standard overlay setup (glOrtho 1000..3000 + glTranslatef(0,0,-2000))
// lands the flat GUI plane at z_ndc = 0 -> the middle of the GS range, leaving
// headroom both ways for the 3D previews (inventory player, item icons). When
// the depth test is off GL forbids depth writes anyway, so the old always-on-top
// constant is kept for painter's-algorithm overlays.
//
// The scale is PS2_GS_Z_MAX, not 0xFFFF: with the signed PSMZ_16S buffer a Z
// above 0x7FFF stores as negative and defeats the depth test entirely, and this
// mapping is the one that put the whole inventory model up there.
static inline int ps2_ortho_gs_z(float oz) {
    if (!s_depthTestEnabled) {
        PS2_DEPTH_STAT(s_dbg_ortho_z_pinned++);
        return PS2_GS_Z_MAX;
    }
    const float range = ps2_projection_state().farPlane - ps2_projection_state().nearPlane;
    if (range == 0.0f) {
        PS2_DEPTH_STAT(s_dbg_ortho_z_pinned++);
        return PS2_GS_Z_MAX;
    }
    const float zndc = (-2.0f * oz - (ps2_projection_state().farPlane + ps2_projection_state().nearPlane)) / range;
    float d = (1.0f - zndc) * 0.5f;
    if (d < 0.0f) d = 0.0f;
    if (d > 1.0f) d = 1.0f;
    const int z = ps2_apply_polygon_depth_bias((int)(d * (float)PS2_GS_Z_MAX));
    PS2_DEPTH_STAT(s_dbg_ortho_z_tested++;
                   if (z < s_dbg_ortho_z_lo) s_dbg_ortho_z_lo = z;
                   if (z > s_dbg_ortho_z_hi) s_dbg_ortho_z_hi = z;
                   if (oz < s_dbg_ortho_oz_lo) s_dbg_ortho_oz_lo = oz;
                   if (oz > s_dbg_ortho_oz_hi) s_dbg_ortho_oz_hi = oz;
                   s_dbg_ortho_near = ps2_projection_state().nearPlane; s_dbg_ortho_far = ps2_projection_state().farPlane);
    return z;
}

// Near/far of the perspective projection, kept for the GS depth mapping.
// Seeded with the values EntityRenderer::setupCameraTransform uses so a draw
// that somehow precedes the first glFrustum still gets a sane Z.
static Ps2DepthMap s_depthMap = {
    64.0f,
    (float)PS2_GS_Z_MAX / (64.0f - 0.05f),
    (float)PS2_GS_Z_MAX * (0.05f * PS2_DEPTH_NEAR_BOOST) * 64.0f /
        (64.0f - 0.05f * PS2_DEPTH_NEAR_BOOST),
    -(float)PS2_GS_Z_MAX * (0.05f * PS2_DEPTH_NEAR_BOOST) /
        (64.0f - 0.05f * PS2_DEPTH_NEAR_BOOST)
};

static void ps2_render_set_frustum_planes(float zNear, float zFar) {
    if (!(zFar > zNear)) return; // degenerate: keep the last good pair
    s_depthMap.zFar  = zFar;
    s_depthMap.scale = (float)PS2_GS_Z_MAX / (zFar - zNear);

    // Depth-mapping near plane. See PS2_DEPTH_NEAR_BOOST: this is the only
    // place the boost is applied, so every projection -- world camera, item
    // preview, sky -- gets the same relative precision gain and none of them
    // can have the boost swallow their whole depth range. Fall back to the
    // projection's own near plane if the boosted one would reach the far
    // plane, which would collapse the mapping entirely.
    float depthNear = zNear * PS2_DEPTH_NEAR_BOOST;
    if (!(depthNear < zFar))
        depthNear = zNear;
    const float depthRange = zFar - depthNear;
    s_depthMap.qScale =  (float)PS2_GS_Z_MAX * depthNear * zFar / depthRange;
    s_depthMap.qBias  = -(float)PS2_GS_Z_MAX * depthNear / depthRange;
}

static void project_cv(const ClipVert& v, float W, float H,
                        float& sx, float& sy, int& sz, float& q) {
    q = 1.0f / v.cw;
    sx = ps2_clamp_guard((v.cx * q + 1.0f) * 0.5f * W, W);
    sy = ps2_clamp_guard((-v.cy * q + 1.0f) * 0.5f * H, H);
    if (s_forceNearZ) { sz = PS2_GS_Z_MAX; return; }
    sz = ps2_apply_polygon_depth_bias(ps2_gs_depth(s_depthMap, v.cz, v.cw, q, (float)PS2_GS_Z_MAX));
}

// The optimized VU0 path is the shipping PS2 3D backend. The generic emitter
// remains as a runtime fallback for unsupported primitive/state combinations.
static bool s_draw_3d_enabled = true;
static bool s_draw_3d_fully_inside = false;

extern "C" void ps2_draw_3d_set_enabled(int enabled) {
	s_draw_3d_enabled = enabled != 0;
	if (!s_draw_3d_enabled)
		s_draw_3d_fully_inside = false;
}

// Per-section hint from WorldRenderer::renderPassCached: this chunk section's
// bounding box is entirely inside all six frustum planes this frame, so the
// optimized path may skip per-vertex clip outcodes for it.
extern "C" void ps2_draw_3d_set_fully_inside(int fullyInside) {
	s_draw_3d_fully_inside = fullyInside != 0;
}

#ifdef PS2_RENDER_STATS
// Batching health and stage cycle counts, owned by Ps2Vu0Draw3D.cpp.
extern "C" {
extern long g_ps2_dbg_strip_flush;
extern long g_ps2_dbg_batch_flush;
extern long g_ps2_dbg_strip_quads;
extern long g_ps2_dbg_clamp_ask;
extern long g_ps2_dbg_clamp_set;
extern long g_ps2_dbg_vu0_quads;
extern unsigned long g_ps2_dbg_cyc_xform;
extern unsigned long g_ps2_dbg_cyc_project;
extern unsigned long g_ps2_dbg_cyc_emit;
#ifdef PS2_ENABLE_VU1_TRANSFORM
extern long g_ps2_dbg_vu1_batches;
extern long g_ps2_dbg_vu1_vertices;
extern unsigned long g_ps2_dbg_cyc_vu1_wait;
#endif
}
#define PS2_HAVE_FASTDRAW_STATS 1
#endif

extern "C" void ps2_dbg_draw_dump() {
#ifdef PS2_RENDER_STATS
    Ps2GsQueueRenderStats queueStats;
    ps2_gs_queue_render_stats(queueStats, false);
    MC_LOG_DEBUG("render", "[PS2] 3D draw: calls=%ld  native=%ld/%ld  prims=%ld  clip=%ld  offscr=%ld  backface=%ld  qflush=%ld"
           "  qpeak=%ldKB qdraw=%ldKB qover=%ld\n",
           s_dbg_3d_calls, s_dbg_native_hits, s_dbg_native_attempts,
           s_dbg_3d_prim, s_dbg_3d_clip, s_dbg_3d_offscreen,
           s_dbg_3d_backface, queueStats.flushes,
           queueStats.peakBytes / 1024, queueStats.maxGuardWriteBytes / 1024, queueStats.guardOverruns);
    Ps2TerrainClusterStats clusterStats;
    ps2_terrain_take_cluster_stats(clusterStats);
    MC_LOG_DEBUG("render", "[PS2] terrain clusters: sections=%ld inside=%ld partial=%ld outside=%ld"
           " culledVerts=%ld guardSafePartial=%ld/%ld guardRisk=%ld/%ld"
           " vu1Ranges=%ld vu1Verts=%ld vu0Batches=%ld vu0Verts=%ld\n",
           clusterStats.sections, clusterStats.insideClusters,
           clusterStats.partialClusters, clusterStats.outsideClusters,
           clusterStats.outsideVertices,
           clusterStats.guardSafePartialClusters,
           clusterStats.guardSafePartialVertices,
           clusterStats.guardNearRiskClusters,
           clusterStats.guardSideRiskClusters,
           clusterStats.vu1Ranges,
           clusterStats.vu1Vertices, clusterStats.vu0GatherBatches,
           clusterStats.vu0GatherVertices);
#ifdef PS2_HAVE_FASTDRAW_STATS
    // quadsPerFlush is the headline: PS2_OPT_STRIP_MAX_VERTS allows 32, so 32
    // means the strip batch is working and ~1 means every quad is paying for its
    // own GIFtag + TEX0 pair. clampSet/clampAsk says whether clamp changes are
    // what is breaking it (the greedy mesher groups quads by atlas tile to keep
    // that ratio low -- see Ps2GreedyMesh.cpp).
    //
    // The three cycle figures are EE cycles at ~294MHz summed over the period,
    // reported as ms so they can be read straight against "render avg". They do
    // not sum to render: everything outside these three blocks (attribute fetch,
    // fog, backface/offscreen tests, gsKit state, the generic path) is the
    // remainder, and a large remainder is itself the answer to where the time is.
    const long stripFlush = g_ps2_dbg_strip_flush;
    MC_LOG_DEBUG("render", "[PS2] batch: stripFlush=%ld batchFlush=%ld quads=%ld quadsPerFlush=%ld"
           "  clampAsk=%ld clampSet=%ld vu0Quads=%ld | cyc xform=%.1fms project=%.1fms emit=%.1fms\n",
           stripFlush, g_ps2_dbg_batch_flush, g_ps2_dbg_strip_quads,
           stripFlush > 0 ? g_ps2_dbg_strip_quads / stripFlush : 0,
           g_ps2_dbg_clamp_ask, g_ps2_dbg_clamp_set, g_ps2_dbg_vu0_quads,
           (double)g_ps2_dbg_cyc_xform   / 294000.0,
           (double)g_ps2_dbg_cyc_project / 294000.0,
           (double)g_ps2_dbg_cyc_emit    / 294000.0);
#ifdef PS2_ENABLE_VU1_TRANSFORM
    MC_LOG_DEBUG("render", "[PS2] vu1 transform: batches=%ld vertices=%ld wait=%.1fms\n",
           g_ps2_dbg_vu1_batches, g_ps2_dbg_vu1_vertices,
           (double)g_ps2_dbg_cyc_vu1_wait / 294000.0);
    g_ps2_dbg_vu1_batches = g_ps2_dbg_vu1_vertices = 0;
    g_ps2_dbg_cyc_vu1_wait = 0;
#endif
    g_ps2_dbg_strip_flush = g_ps2_dbg_batch_flush = g_ps2_dbg_strip_quads = 0;
    g_ps2_dbg_clamp_ask = g_ps2_dbg_clamp_set = 0;
    g_ps2_dbg_vu0_quads = 0;
    g_ps2_dbg_cyc_xform = g_ps2_dbg_cyc_project = g_ps2_dbg_cyc_emit = 0;
#endif
#ifdef PS2_ENABLE_VU1_TERRAIN
    Ps2Vu1TerrainStats terrainVu1;
    ps2_vu1_terrain_take_stats(terrainVu1);
    MC_LOG_DEBUG("render", "[PS2] terrain VU1: pages=%ld batches=%ld vertices=%ld clipped=%ld/%ld"
           " probe=%ld/%ld retry=%ld xgkicks=%ld"
           " buffers=%ld/%ld qwords=%ld pagePeak=%ld waits=%ld wait=%.1fms transitions=%ld"
           " canary=%ld/%ld fail=%ld\n",
           terrainVu1.pages, terrainVu1.batches, terrainVu1.vertices,
           terrainVu1.clippedBatches, terrainVu1.clippedVertices,
           terrainVu1.clippedProbeBatches, terrainVu1.clippedProbeVertices,
           terrainVu1.clippedProbeRetries,
           terrainVu1.xgkicks, terrainVu1.buffer0Batches,
           terrainVu1.buffer1Batches, terrainVu1.qwords, terrainVu1.maxPageQwords,
           terrainVu1.waits, (double)terrainVu1.waitCycles / 294000.0,
           terrainVu1.pathTransitions, terrainVu1.canaryCompleted,
           terrainVu1.canarySubmitted, terrainVu1.canaryFailures);
#endif
    s_dbg_3d_calls = s_dbg_3d_prim = s_dbg_3d_clip = s_dbg_3d_offscreen = 0;
    s_dbg_native_attempts = s_dbg_native_hits = 0;
    s_dbg_3d_backface = 0;
    ps2_gs_queue_render_stats(queueStats, true);
#endif
}

// Depth state actually reached the GS this period. Read it with the inventory
// open to tell the four "broken z-buffer" causes apart:
//   zclr=0            -> glClear(kClearDepthBit) never ran, so the preview
//                        is testing against whatever the world left behind.
//   ztest=0/zpin>0    -> every ortho vertex took the always-on-top constant,
//                        i.e. kCapDepthTest was off for the whole GUI pass and
//                        the model can only sort by submission order.
//   zwr=0             -> the test is on but ZMSK masked every write, so no
//                        fragment ever updates the buffer and nothing occludes.
//   zlo..zhi narrow   -> the test and writes are live but the model is squeezed
//                        into too few levels to separate its own limbs. Compare
//                        the span against PS2_GS_Z_MAX: the GUI plane sits at
//                        half of it, and a model 30 units deep should cover a
//                        few hundred levels.
extern "C" void ps2_dbg_depth_dump() {
#ifdef PS2_RENDER_STATS
    MC_LOG_DEBUG("render", "[PS2] depth: zclr=%ld ztest=%ld zpin=%ld zwr=%ld/%ld zlo=%d zhi=%d span=%d (max=%d)\n",
           s_dbg_depth_clears, s_dbg_ortho_z_tested, s_dbg_ortho_z_pinned,
           s_dbg_zwrite_on, s_dbg_zwrite_on + s_dbg_zwrite_off,
           s_dbg_ortho_z_hi < 0 ? -1 : s_dbg_ortho_z_lo, s_dbg_ortho_z_hi,
           s_dbg_ortho_z_hi < 0 ? 0 : (s_dbg_ortho_z_hi - s_dbg_ortho_z_lo),
           PS2_GS_Z_MAX);
    // The eye Z that produced those, against the glOrtho range that maps it.
    // setupOverlayRendering builds glOrtho(1000,3000) + glTranslatef(0,0,-2000),
    // so a GUI vertex should arrive at oz = -2000 and land mid-range; anything
    // above -1000 (or below -3000) clamps and reports as a collapsed span.
    MC_LOG_DEBUG("render", "[PS2] depth: oz=%.1f..%.1f ortho near=%.1f far=%.1f (expect oz ~ -2000)\n",
           s_dbg_ortho_oz_hi < -1e29f ? 0.0f : s_dbg_ortho_oz_lo,
           s_dbg_ortho_oz_hi < -1e29f ? 0.0f : s_dbg_ortho_oz_hi,
           s_dbg_ortho_near, s_dbg_ortho_far);
    // Sprite share of the GUI. Every quad the sprite emitter takes is 4 fewer
    // vertices and one fewer primitive on the GS; a high spr:tri ratio is what
    // makes that emitter worth its qualification test.
    MC_LOG_DEBUG("render", "[PS2] gui: sprites=%ld tris=%ld\n",
           s_dbg_ortho_sprites, s_dbg_ortho_sprite_tri);
    s_dbg_ortho_sprites = s_dbg_ortho_sprite_tri = 0;
    s_dbg_depth_clears = s_dbg_ortho_z_tested = s_dbg_ortho_z_pinned = 0;
    s_dbg_zwrite_on = s_dbg_zwrite_off = 0;
    s_dbg_ortho_z_lo = 0x7FFFFFFF;
    s_dbg_ortho_z_hi = -1;
    s_dbg_ortho_oz_lo = 1e30f;
    s_dbg_ortho_oz_hi = -1e30f;
#endif
}

// Program the GS registers a draw call depends on.
//
// Every call in here pushes an A+D register packet into the same unchecked
// queue as the geometry, so the CALLER must have reserved for them already
// (ps2_gs_queue_guard_prim). That ordering used to be inverted, which left
// these ~96 bytes covered only by whatever slack the previous draw happened to
// leave behind.
static void ps2_draw_apply_gs_state() {
    ps2_apply_depth_state();
    ps2_apply_color_mask();
    ps2_apply_texa();

    // Keep this write explicit. Startup/menu/cursor code can touch
    // gsGlobal->PrimAlphaEnable directly, so caching it here can leave stale
    // blend state and break PNG transparency in GUI draws.
    // Transparency on this GS path is driven by GS alpha blending (PrimAlpha).
    // Anything that wants per-texel transparency — explicit kCapBlend draws AND
    // alpha-test cutouts (2D items, particles, model skin overlay, terrain) —
    // must enable it. With our 1-bit texture alpha, SRC_ALPHA blend reproduces
    // the binary cutout (A=0 -> background, A=1 -> opaque) reliably, which the
    // GS alpha-test register alone did not on real hardware (black squares).
    const bool wantAlpha = st.blend || st.alphaTest;
    gsGlobal->PrimAlphaEnable = wantAlpha ? GS_SETTING_ON : GS_SETTING_OFF;
    s_primAlphaEnableValid = true;
    s_primAlphaEnable = wantAlpha;
    if (st.blend) {
        s_blendAlphaValid = false;
        ps2_apply_blend();
    } else if (st.alphaTest) {
        // Cutout-only: force standard (Cs-Cd)*As/128+Cd blend.
        s_blendAlphaValid = false;
        ps2_set_blend_alpha(GS_SETREG_ALPHA(0,1,0,1,0));
    }

    // Also drive the real GS alpha test: discarded texels write no color/Z, so
    // cutouts keep correct depth (opaque texels still blend to themselves).
    ps2_apply_alpha_test();
}

// Resolve the bound texture to the GSTEXTURE this draw should sample, or
// nullptr for an untextured draw. Also the point where a deferred
// ps2_texture_resolve() also flushes any deferred sub-image upload, so an
// animated atlas is re-sent once here rather than once per touched tile.
static GSTEXTURE* ps2_draw_resolve_texture(bool hasTexCoords) {
    if (!(st.tex2d && hasTexCoords))
        return nullptr;
    return ps2_texture_resolve(ps2_texture_bound_name());
}

static Ps2RenderState ps2_capture_render_state(bool allowCull, bool allowFog)
{
    if (s_lightingOn && s_nen && s_np != nullptr)
        ps2_prepare_normal_matrix();
    Ps2RenderState render;
    render.cullFace = allowCull && st.cullFace;
    render.frontFaceCCW = st.frontFace == kFaceCCW;
    render.cullBackFace = st.cullMode == kFaceBack;
    render.flatR = st.cr;
    render.flatG = st.cg;
    render.flatB = st.cb;
    render.flatA = st.ca;
    render.fogEnabled = allowFog && st.fog;
    render.fogMode = st.fogMode;
    render.fogDensity = st.fogDensity;
    render.fogStart = st.fogStart;
    render.fogEnd = st.fogEnd;
    render.fogR = st.fogR;
    render.fogG = st.fogG;
    render.fogB = st.fogB;
    render.fogLinearValid = st.fogEnd > st.fogStart;
    render.fogLinearScale = render.fogLinearValid ? 1.0f / (st.fogEnd - st.fogStart) : 0.0f;
    render.fogLinearBias = st.fogEnd * render.fogLinearScale;
    render.smoothShading = s_shadeModel == kShadeSmooth;
    render.lightVertex = (s_lightingOn && s_nen && s_np != nullptr) ? ps2_light_vertex : nullptr;
    return render;
}


static int ps2_texture_log2(unsigned int value)
{
    int power = 0;
    while ((1u << power) < value)
        ++power;
    return power;
}

bool ps2_render_prepare_terrain_gpu_state(Ps2TerrainGpuState& out)
{
    if (!gsGlobal || !gsGlobal->Test || ps2_projection_is_orthographic() || !st.tex2d)
        return false;
    if (st.fog && st.fogMode != kFogLinear)
        return false;

    // Apply TEST/Z/FRAME/ALPHA state and resolve deferred texture updates while
    // we still own Path3. Reserve queue slack before any of those register
    // writes: terrain pass setup is not reached through the normal draw guard.
    // ps2_render_acquire_path1() drains this queue before VIF1 emits Path1 data.
    ps2_gs_queue_guard(0);
    ps2_draw_apply_gs_state();
    GSTEXTURE* texture = ps2_draw_resolve_texture(true);
    if (!texture || texture->Width <= 0 || texture->Height <= 0)
        return false;

    const int tw = ps2_texture_log2((unsigned int)texture->Width);
    const int th = ps2_texture_log2((unsigned int)texture->Height);
    if (texture->VramClut == 0) {
        out.tex0 = GS_SETREG_TEX0(texture->Vram / 256, texture->TBW, texture->PSM,
                                 tw, th, gsGlobal->PrimAlphaEnable, 0,
                                 0, 0, 0, 0, 0);
    } else {
        out.tex0 = GS_SETREG_TEX0(texture->Vram / 256, texture->TBW, texture->PSM,
                                 tw, th, gsGlobal->PrimAlphaEnable, 0,
                                 texture->VramClut / 256, texture->ClutPSM,
                                 texture->ClutStorageMode, 0, 1);
    }
    out.texa = GS_SETREG_TEXA(0x00, 1, 0x80);
    out.alpha = s_blendAlphaValid
        ? s_blendAlphaReg
        : GS_SETREG_ALPHA(0, 1, 0, 1, 0);

    out.textureWidth = texture->Width;
    out.textureHeight = texture->Height;
    out.primContext = gsGlobal->PrimContext;
    const GSTEST* test = gsGlobal->Test;
    out.test = GS_SETREG_TEST(test->ATE, test->ATST, test->AREF, test->AFAIL,
                              test->DATE, test->DATM, test->ZTE, test->ZTST);
    out.zbuf = GS_SETREG_ZBUF(gsGlobal->ZBuffer / 8192, gsGlobal->PSMZ, s_gsZmsk);
    out.primAlphaEnable = gsGlobal->PrimAlphaEnable;
    out.primAAEnable = gsGlobal->PrimAAEnable;
    out.offsetX = gsGlobal->OffsetX;
    out.offsetY = gsGlobal->OffsetY;
    out.viewW = ps2_viewport_width();
    out.viewH = ps2_viewport_height();
    out.depthQScale = s_depthMap.qScale;
    out.depthQBias = s_depthMap.qBias + ps2_polygon_depth_bias_gs();
    out.forceNearZ = s_forceNearZ;
    out.render = ps2_capture_render_state(true, true);
    return true;
}

void ps2_render_invalidate_path1_state()
{
    s_gsZtst = 0xFF;
    s_gsZmsk = 0xFF;
    s_clampMode = -1;
    s_regionUFix = -1;
    s_regionVFix = -1;
    s_blendAlphaValid = false;
    s_alphaTestValid = false;
    s_primAlphaEnableValid = false;
    s_texaValid = false;
    s_gsFrameMaskValid = false;
}

void ps2_render_invalidate_framebuffer_state()
{
    // gsKit_sync_flip() keeps PrimContext unchanged. Its gsKit_setactive()
    // call only rewrites FRAME_1/FRAME_2 (and SCISSOR) so both contexts target
    // the new draw buffer, with FBMSK reset to zero. The other cached GS state
    // remains valid across a normal flip; forcing it unknown every frame only
    // emits redundant register traffic.
    s_gsFrameMaskValid = false;
}

static bool ps2_try_draw_3d(unsigned int mode, int first, int count,
                            GSTEXTURE* tex, const Mat4 mvp,
                            bool packedTerrain,
                            const Ps2NativeClampRun* clampRuns,
                            int clampRunCount,
                            const void* vertices, int vertexStride, int vertexSize,
                            const void* texCoords, int texCoordStride, bool texCoordEnabled,
                            const void* colors, int colorStride, int colorSize,
                            bool colorEnabled, bool colorFloat,
                            bool fullyInside) {
	if (!s_draw_3d_enabled || (mode != kPrimTriangles && mode != kPrimQuads)
            || ps2_projection_is_orthographic())
        return false;

	Ps2Draw3DState fastState;
	fastState.packedTerrain = packedTerrain;
    fastState.clampRuns = clampRuns;
    fastState.clampRunCount = clampRunCount;
    fastState.tileAtlas = ps2_texture_bound_is_tile_atlas();
    fastState.quads = (mode == kPrimQuads);
    fastState.gsGlobal = gsGlobal;
    fastState.texture = tex;
    fastState.mvp = mvp;
    fastState.vertices = vertices;
    fastState.vertexStride = vertexStride;
    fastState.vertexSize = vertexSize;
    fastState.texCoords = texCoords;
    fastState.texCoordStride = texCoordStride;
    fastState.texCoordEnabled = texCoordEnabled;
    fastState.colors = colors;
    fastState.colorStride = colorStride;
    fastState.colorSize = colorSize;
    fastState.colorEnabled = colorEnabled;
    fastState.colorFloat = colorFloat;
    fastState.render = ps2_capture_render_state(true, true);
    fastState.first = first;
    fastState.count = count;
    fastState.ortho = false;
    fastState.fullyInside = fullyInside;
    fastState.depth = s_depthMap;
    fastState.depth.qBias += ps2_polygon_depth_bias_gs();
    fastState.forceNearZ = s_forceNearZ;
    fastState.viewW = ps2_viewport_width();
    fastState.viewH = ps2_viewport_height();
#ifdef PS2_RENDER_STATS
    fastState.debugPrims = &s_dbg_3d_prim;
    fastState.debugClipped = &s_dbg_3d_clip;
    fastState.debugOffscreen = &s_dbg_3d_offscreen;
    fastState.debugBackface = &s_dbg_3d_backface;
#else
    fastState.debugPrims = nullptr;
    fastState.debugClipped = nullptr;
    fastState.debugOffscreen = nullptr;
    fastState.debugBackface = nullptr;
#endif
    fastState.setTextureClampForUv = ps2_set_texture_clamp_for_uv;
    fastState.applyClampSel = ps2_apply_clamp_sel;
	return ps2_draw_3d(fastState);
}

static bool ps2_native_draw_ortho_mesh(const Ps2NativeMeshView& mesh) {
    const unsigned int mode = (unsigned int)mesh.drawMode;
    if (!gsGlobal || !ps2_projection_is_orthographic() || !mesh.vertices || mesh.count < 1)
        return false;
    if (mode != kPrimQuads && mode != kPrimTriangles && mode != kPrimTriangleStrip)
        return false;
    if (mesh.colorEnabled && !mesh.colors)
        return false;
    if (mesh.hasNormals && mesh.normals == nullptr)
        return false;

    const void* oldNp = s_np;
    const int oldNstride = s_nstride;
    const unsigned int oldNtype = s_ntype;
    const bool oldNen = s_nen;

    s_nen = mesh.hasNormals && mesh.normals != nullptr;
    if (s_nen) {
        s_np = mesh.normals;
        s_nstride = (int)mesh.normalStride;
        s_ntype = mesh.normalFloat ? kTypeFloat : kTypeByte;
    } else {
        s_np = nullptr;
        s_nstride = 0;
        s_ntype = kTypeByte;
    }

    gsGlobal->ZBuffering = GS_SETTING_ON;
    ps2_gs_queue_guard_prim(mode, mesh.count);
    ps2_draw_apply_gs_state();

    Ps2Draw2DState drawState;
    drawState.gsGlobal = gsGlobal;
    drawState.texture = ps2_draw_resolve_texture(mesh.texCoordEnabled && mesh.texCoords != nullptr);
	drawState.modelView = ps2_matrix_model_view();
    drawState.render = ps2_capture_render_state(false, false);
    drawState.left = ps2_projection_state().left;
    drawState.right = ps2_projection_state().right;
    drawState.bottom = ps2_projection_state().bottom;
    drawState.top = ps2_projection_state().top;
    drawState.nearPlane = ps2_projection_state().nearPlane;
    drawState.farPlane = ps2_projection_state().farPlane;
    drawState.viewW = ps2_viewport_width();
    drawState.viewH = ps2_viewport_height();
    drawState.depthMax = PS2_GS_Z_MAX;
    drawState.depthTest = s_depthTestEnabled;
    drawState.queueGuard = ps2_gs_queue_guard;
    drawState.applyClampSel = ps2_apply_clamp_sel;
#ifdef PS2_RENDER_STATS
    drawState.debugSprites = &s_dbg_ortho_sprites;
    drawState.debugSpriteFallbacks = &s_dbg_ortho_sprite_tri;
    drawState.debugDepthTested = &s_dbg_ortho_z_tested;
    drawState.debugDepthPinned = &s_dbg_ortho_z_pinned;
    drawState.debugDepthLow = &s_dbg_ortho_z_lo;
    drawState.debugDepthHigh = &s_dbg_ortho_z_hi;
    drawState.debugEyeZLow = &s_dbg_ortho_oz_lo;
    drawState.debugEyeZHigh = &s_dbg_ortho_oz_hi;
    drawState.debugNear = &s_dbg_ortho_near;
    drawState.debugFar = &s_dbg_ortho_far;
#endif

    const bool consumed = ps2_draw_2d_mesh(mesh, drawState);

    s_np = oldNp;
    s_nstride = oldNstride;
    s_ntype = oldNtype;
    s_nen = oldNen;
    return consumed;
}

static void ps2_backend_draw_arrays(unsigned int mode, int first, int count,
                                    const float* preparedMvp);

static bool ps2_backend_mode_supported(unsigned int mode) {
    return mode == kPrimPoints || mode == kPrimLines || mode == kPrimLineLoop ||
           mode == kPrimLineStrip || mode == kPrimTriangles ||
           mode == kPrimTriangleStrip || mode == kPrimTriangleFan ||
           mode == kPrimQuads;
}

struct Ps2BackendArrayState {
    const void* vp; int vstride; int vsize;
    const void* tp; int tstride;
    const void* cp; int cstride; int csize; bool cfloat;
    bool ven; bool ten; bool cen;
    const void* np; int nstride; unsigned int ntype; bool nen;
};

static bool ps2_backend_draw_mesh_generic(const Ps2NativeMeshView& mesh,
                                          const float* preparedMvp,
                                          bool allowLighting) {
    const unsigned int mode = (unsigned int)mesh.drawMode;
	if (!gsGlobal || mesh.packedTerrain || !mesh.vertices || mesh.count <= 0 ||
		!ps2_backend_mode_supported(mode))
        return false;
    if (mesh.colorEnabled && mesh.colors == nullptr)
        return false;
    if (mesh.texCoordEnabled && mesh.texCoords == nullptr)
        return false;
    if (s_lightingOn && (!allowLighting || !mesh.hasNormals || mesh.normals == nullptr))
        return false;

    Ps2BackendArrayState old = {
        st.vp, st.vstride, st.vsize, st.tp, st.tstride,
        st.cp, st.cstride, st.csize, st.cfloat,
        st.ven, st.ten, st.cen, s_np, s_nstride, s_ntype, s_nen
    };

    st.vp = mesh.vertices;
    st.vstride = mesh.vertexStride;
    st.vsize = mesh.vertexSize;
    st.ven = true;
    st.tp = mesh.texCoords;
    st.tstride = mesh.texCoordStride;
    st.ten = mesh.texCoordEnabled;
    st.cp = mesh.colors;
    st.cstride = mesh.colorStride;
    st.csize = mesh.colorSize;
    st.cfloat = mesh.colorFloat;
    st.cen = mesh.colorEnabled;
    s_np = mesh.normals;
    s_nstride = mesh.normalStride;
    s_ntype = mesh.normalFloat ? kTypeFloat : kTypeByte;
    s_nen = allowLighting && mesh.hasNormals && mesh.normals != nullptr;

    ps2_backend_draw_arrays(mode, mesh.first, mesh.count, preparedMvp);

    st.vp = old.vp; st.vstride = old.vstride; st.vsize = old.vsize;
    st.tp = old.tp; st.tstride = old.tstride;
    st.cp = old.cp; st.cstride = old.cstride; st.csize = old.csize; st.cfloat = old.cfloat;
    st.ven = old.ven; st.ten = old.ten; st.cen = old.cen;
    s_np = old.np; s_nstride = old.nstride; s_ntype = old.ntype; s_nen = old.nen;
    return true;
}

static bool ps2_native_mesh_state_supported(const Ps2NativeMeshView& mesh, bool allowLighting) {
    const unsigned int mode = (unsigned int)mesh.drawMode;
    if (!gsGlobal || !mesh.vertices || mesh.count < 1)
        return false;
	if (!s_draw_3d_enabled || (mode != kPrimTriangles && mode != kPrimQuads)
            || ps2_projection_is_orthographic())
        return false;
    // Prepared terrain contexts carry only the final MVP, not the modelview
    // needed to transform normals. Direct model draws may use the live
    // modelview/light state when an explicit normal stream is available.
    if (s_lightingOn && (!allowLighting || !mesh.hasNormals || mesh.normals == nullptr))
        return false;
    return true;
}

bool ps2_native_prepare_frame_context(Ps2NativeFrameContext& out) {
    out.valid = false;
    out.viewW = 0.0f;
    out.viewH = 0.0f;
	if (!gsGlobal || ps2_projection_is_orthographic() || !s_draw_3d_enabled)
        return false;

	ps2_matrix_multiply(out.mvp, ps2_matrix_projection(), ps2_matrix_model_view());
    out.viewW = ps2_viewport_width();
    out.viewH = ps2_viewport_height();
    out.valid = true;
    return true;
}

bool ps2_native_prepare_translated_context(Ps2NativeDrawContext& out,
                                           const Ps2NativeFrameContext& frame,
                                           float tx, float ty, float tz,
                                           bool fullyInside) {
    out.valid = false;
    out.fullyInside = fullyInside;
    if (!frame.valid)
        return false;

    // Appending T to the already prepared camera MVP does not need another 4x4
    // multiply: for M*T only the fourth column changes.
    __builtin_memcpy(out.mvp, frame.mvp, sizeof(out.mvp));
    out.mvp[12] += out.mvp[0] * tx + out.mvp[4] * ty + out.mvp[8]  * tz;
    out.mvp[13] += out.mvp[1] * tx + out.mvp[5] * ty + out.mvp[9]  * tz;
    out.mvp[14] += out.mvp[2] * tx + out.mvp[6] * ty + out.mvp[10] * tz;
    out.mvp[15] += out.mvp[3] * tx + out.mvp[7] * ty + out.mvp[11] * tz;
    out.valid = true;
    return true;
}

static bool ps2_native_draw_mesh_with_context(const Ps2NativeMeshView& mesh,
                                              const Ps2NativeDrawContext& context,
                                              bool allowLighting) {
    PS2_RENDER_STAT(s_dbg_native_attempts++);
    if (!context.valid || !ps2_native_mesh_state_supported(mesh, allowLighting))
        return false;

    const void* oldNp = s_np;
    const int oldNstride = s_nstride;
    const unsigned int oldNtype = s_ntype;
    const bool oldNen = s_nen;
    const bool useNativeNormals = allowLighting && s_lightingOn && mesh.hasNormals;
    if (useNativeNormals) {
        s_np = mesh.normals;
        s_nstride = (int)mesh.normalStride;
        s_ntype = mesh.normalFloat ? kTypeFloat : kTypeByte;
        s_nen = true;
    }

    const unsigned int mode = (unsigned int)mesh.drawMode;
    gsGlobal->ZBuffering = GS_SETTING_ON;
    PS2_RENDER_STAT(s_dbg_3d_calls++);
    ps2_gs_queue_guard_prim(mode, mesh.count);
    ps2_draw_apply_gs_state();

    GSTEXTURE* tex = ps2_draw_resolve_texture(mesh.texCoordEnabled && mesh.texCoords != nullptr);
	const bool consumed = ps2_try_draw_3d(mode, mesh.first, mesh.count, tex, context.mvp,
												mesh.packedTerrain,
                                                    mesh.clampRuns, mesh.clampRunCount,
                                                    mesh.vertices, mesh.vertexStride, mesh.vertexSize,
                                                    mesh.texCoords, mesh.texCoordStride, mesh.texCoordEnabled,
                                                    mesh.colors, mesh.colorStride, mesh.colorSize,
                                                    mesh.colorEnabled, mesh.colorFloat,
                                                    context.fullyInside);

    if (useNativeNormals) {
        s_np = oldNp;
        s_nstride = oldNstride;
        s_ntype = oldNtype;
        s_nen = oldNen;
    }

    PS2_RENDER_STAT(if (consumed) s_dbg_native_hits++);
    return consumed;
}

bool ps2_native_draw_mesh_prepared(const Ps2NativeMeshView& mesh,
                                   const Ps2NativeDrawContext& context) {
    if (!context.valid)
        return false;
    if (ps2_native_draw_mesh_with_context(mesh, context, false))
        return true;
    return ps2_backend_draw_mesh_generic(mesh, context.mvp, false);
}

bool ps2_native_draw_mesh(const Ps2NativeMeshView& mesh) {
    if (ps2_projection_is_orthographic()) {
        PS2_RENDER_STAT(s_dbg_native_attempts++);
        if (ps2_native_draw_ortho_mesh(mesh)) {
            PS2_RENDER_STAT(s_dbg_native_hits++);
            return true;
        }
        return ps2_backend_draw_mesh_generic(mesh, nullptr, true);
    }

    Ps2NativeDrawContext context;
    context.valid = true;
	context.fullyInside = s_draw_3d_fully_inside;
	ps2_matrix_multiply(context.mvp, ps2_matrix_projection(), ps2_matrix_model_view());
    if (ps2_native_draw_mesh_with_context(mesh, context, true))
        return true;
    return ps2_backend_draw_mesh_generic(mesh, nullptr, true);
}

static void ps2_backend_draw_arrays(unsigned int mode, int first, int count, const float* preparedMvp) {
    if (!gsGlobal || !st.vp || count < 1) return;
    // Depth behaviour is driven entirely through the TEST/ZBUF registers
    // (ps2_apply_depth_state); keep the gsKit flag pinned ON so gsKit_clear
    // and any internal gsKit path stay consistent with the init-time setup.
    gsGlobal->ZBuffering = GS_SETTING_ON;
    if (!ps2_projection_is_orthographic()) PS2_RENDER_STAT(s_dbg_3d_calls++);

    // Reserve FIRST, then program state — see ps2_draw_apply_gs_state. Doing it
    // up front also means the exec+reset the guard may trigger cannot land
    // between two register writes meant to apply to the same primitive.
    ps2_gs_queue_guard_prim(mode, count);
    ps2_draw_apply_gs_state();

    GSTEXTURE* tex = ps2_draw_resolve_texture(st.tp != nullptr && st.ten);
    const Ps2RenderState renderState = ps2_capture_render_state(true, true);

    // Resolved once per draw call instead of per vertex/per triangle.
    const Ps2ArrayCursor vc = vp_cursor();
    const Ps2ArrayCursor tc = tp_cursor();
    const float texW = tex ? (float)tex->Width  : 1.0f;
    const float texH = tex ? (float)tex->Height : 1.0f;

    // Build MVP once per draw call for 3D mode.
    // Also build row-major VU_MATRIX copy for Vu0ApplyMatrix (VU0 macro mode HW transform).
    Mat4 mvp;
    VU_MATRIX vu_mvp;
    if (!ps2_projection_is_orthographic()) {
        if (preparedMvp)
			ps2_matrix_copy(mvp, preparedMvp);
        else
			ps2_matrix_multiply(mvp, ps2_matrix_projection(), ps2_matrix_model_view());
        vu_mvp = mat4_to_vu(mvp);
    }

    const float scrW = ps2_viewport_width(), scrH = ps2_viewport_height();
    const float gbx = ps2_guard_clip_scale(scrW);
    const float gby = ps2_guard_clip_scale(scrH);

    // Build clip-space vertex from array index i (3D only — do not call for ortho).
    auto make_cv = [&](int i) -> ClipVert {
        const float* v = at_(vc, i);
        VU_VECTOR vert __attribute__((aligned(16))) = {v[0], v[1], (st.vsize>=3?v[2]:0.0f), 1.0f};
        VU_VECTOR clip __attribute__((aligned(16)));
        Vu0ApplyMatrix(&vu_mvp, &vert, &clip);
        ClipVert cv;
        cv.cx = clip.x; cv.cy = clip.y; cv.cz = clip.z; cv.cw = clip.w;
        if (tex && st.tp && st.ten) { const float* t=at_(tc, i); cv.u=t[0]; cv.v=t[1]; }
        else { cv.u = cv.v = 0.0f; }
        getcol(i, cv.r, cv.g, cv.bl, cv.a);
        if (renderState.fogEnabled) {
            const float f = ps2_render_fog_factor(renderState, clip.w);
            const float inv = 1.0f - f;
            cv.r  = (u8)(f * cv.r  + inv * renderState.fogR * 255.0f);
            cv.g  = (u8)(f * cv.g  + inv * renderState.fogG * 255.0f);
            cv.bl = (u8)(f * cv.bl + inv * renderState.fogB * 255.0f);
        }
        return cv;
    };

    // Clip triangle against the near + guard-band planes (outcode trivial
    // accept/reject first) and submit all resulting triangles to GS.
    auto submit_clip_tri = [&](const ClipVert tri[3]) {
        int oc0 = ps2_clip_outcode(tri[0].cx, tri[0].cy, tri[0].cz, tri[0].cw, gbx, gby);
        int oc1 = ps2_clip_outcode(tri[1].cx, tri[1].cy, tri[1].cz, tri[1].cw, gbx, gby);
        int oc2 = ps2_clip_outcode(tri[2].cx, tri[2].cy, tri[2].cz, tri[2].cw, gbx, gby);
        if (oc0 & oc1 & oc2) { PS2_RENDER_STAT(s_dbg_3d_clip++); return; }

        ClipVert poly[PS2_CLIP_MAX_POLY];
        poly[0] = tri[0]; poly[1] = tri[1]; poly[2] = tri[2];
        int n = 3;
        int mask = oc0 | oc1 | oc2;
        if (mask) {
            n = ps2_clip_poly_guard(poly, 3, mask, gbx, gby);
            if (n == 0) { PS2_RENDER_STAT(s_dbg_3d_clip++); return; }
        }
        for (int k = 1; k + 1 < n; k++) {
            float x0,y0,x1,y1,x2,y2; float q0,q1,q2; int z0,z1,z2;
            project_cv(poly[0],   scrW, scrH, x0, y0, z0, q0);
            project_cv(poly[k],   scrW, scrH, x1, y1, z1, q1);
            project_cv(poly[k+1], scrW, scrH, x2, y2, z2, q2);
            if (ps2_tri_offscreen(x0,y0,x1,y1,x2,y2,scrW,scrH)) { PS2_RENDER_STAT(s_dbg_3d_offscreen++); continue; }
            if (is_culled_3d(x0,y0,x1,y1,x2,y2)) { PS2_RENDER_STAT(s_dbg_3d_backface++); continue; }
            PS2_RENDER_STAT(s_dbg_3d_prim++);
            const ClipVert& c0=poly[0]; const ClipVert& c1=poly[k]; const ClipVert& c2=poly[k+1];
            const ClipVert& s0 = renderState.smoothShading ? c0 : c2;
            const ClipVert& s1 = renderState.smoothShading ? c1 : c2;
            if (tex) {
                ps2_set_texture_clamp_for_uv(tex,
                    c0.u*(float)tex->Width, c0.v*(float)tex->Height,
                    c1.u*(float)tex->Width, c1.v*(float)tex->Height,
                    c2.u*(float)tex->Width, c2.v*(float)tex->Height);
                GSPRIMSTQPOINT pts[3];
                pts[0].rgbaq=color_to_RGBAQ(PS2_TEXCOL(s0.r),PS2_TEXCOL(s0.g),PS2_TEXCOL(s0.bl),(u8)(s0.a>>1),q0); pts[0].stq=vertex_to_STQ(c0.u*q0,c0.v*q0); pts[0].xyz2=vertex_to_XYZ2(gsGlobal,x0,y0,z0);
                pts[1].rgbaq=color_to_RGBAQ(PS2_TEXCOL(s1.r),PS2_TEXCOL(s1.g),PS2_TEXCOL(s1.bl),(u8)(s1.a>>1),q1); pts[1].stq=vertex_to_STQ(c1.u*q1,c1.v*q1); pts[1].xyz2=vertex_to_XYZ2(gsGlobal,x1,y1,z1);
                pts[2].rgbaq=color_to_RGBAQ(PS2_TEXCOL(c2.r),PS2_TEXCOL(c2.g),PS2_TEXCOL(c2.bl),(u8)(c2.a>>1),q2); pts[2].stq=vertex_to_STQ(c2.u*q2,c2.v*q2); pts[2].xyz2=vertex_to_XYZ2(gsGlobal,x2,y2,z2);
                ps2_gs_queue_guard(3);
                gsKit_prim_list_triangle_goraud_texture_stq_3d(gsGlobal, tex, 3, pts);
            } else {
                ps2_gs_queue_guard(3);
                gsKit_prim_triangle_gouraud_3d(gsGlobal, x0,y0,z0, x1,y1,z1, x2,y2,z2,
                    mkcol(s0.r,s0.g,s0.bl,s0.a),
                    mkcol(s1.r,s1.g,s1.bl,s1.a),
                    mkcol(c2.r,c2.g,c2.bl,c2.a));
            }
        }
    };

    // Clip one 3D line segment and submit it as a GS LINE primitive.
    //
    // Lines get their own clipper rather than borrowing the polygon one -- see
    // ps2_clip_segment_guard in Ps2ClipGuard.h. They are also never textured:
    // gsKit's goraud line REGLIST carries only PRIM/RGBAQ/XYZ2, so the GS
    // rasterises them with TME off, which is exactly what the two callers
    // (the block selection box and the fishing line) ask for anyway.
    auto submit_clip_line = [&](ClipVert a, ClipVert b) {
        const int oca = ps2_clip_outcode(a.cx, a.cy, a.cz, a.cw, gbx, gby);
        const int ocb = ps2_clip_outcode(b.cx, b.cy, b.cz, b.cw, gbx, gby);
        if (oca & ocb) { PS2_RENDER_STAT(s_dbg_3d_clip++); return; }

        const int mask = oca | ocb;
        if (mask && !ps2_clip_segment_guard(a, b, mask, gbx, gby)) {
            PS2_RENDER_STAT(s_dbg_3d_clip++);
            return;
        }

        float x0, y0, x1, y1, q0, q1;
        int z0, z1;
        project_cv(a, scrW, scrH, x0, y0, z0, q0);
        project_cv(b, scrW, scrH, x1, y1, z1, q1);
        PS2_RENDER_STAT(s_dbg_3d_prim++);
        ps2_gs_queue_guard(2);
        gsKit_prim_line_goraud_3d(gsGlobal, x0, y0, z0, x1, y1, z1,
            renderState.smoothShading ? mkcol(a.r, a.g, a.bl, a.a) : mkcol(b.r, b.g, b.bl, b.a),
            mkcol(b.r, b.g, b.bl, b.a));
    };

		if (ps2_try_draw_3d(mode, first, count, tex, mvp,
									false,
                                     nullptr, 0,
                                     st.vp, st.vstride, st.vsize,
                                 st.tp, st.tstride, st.ten,
                                 st.cp, st.cstride, st.csize, st.cen, st.cfloat,
								 s_draw_3d_fully_inside))
        return;

    // Screen coords + GS depth for an ortho vertex.
    //
    // Ortho GUI still uses the MODELVIEW stack for translated/scaled/rotated
    // item icons and the inventory player preview. The old PS2 path ignored
    // MODELVIEW in ortho mode, so those models were submitted near 0,0 or
    // behind the inventory background.
    //
    // q is pinned to 1: there is no perspective divide in ortho, so the STQ
    // coordinates are the raw UVs. This used to be a combined 3D/ortho helper
    // whose 3D half no draw path could reach (the 3D modes all go through
    // make_cv/submit_clip_tri), plus a second, entirely unreferenced copy of
    // the same code.
    auto svq = [&](int i, float& sx, float& sy, int& sz) {
        const float* v = at_(vc, i);
        float ox, oy, oz, ow;
		ps2_matrix_transform_point(ps2_matrix_model_view(),
			v[0], v[1], (st.vsize >= 3 ? v[2] : 0.0f), ox, oy, oz, ow);
        if (fabsf(ow) > 1e-6f) { ox /= ow; oy /= ow; oz /= ow; }
        sx = gsx(ox); sy = gsy(oy); sz = ps2_ortho_gs_z(oz);
    };

    // ---- Ortho (GUI/HUD) emission ----
    //
    // One shared emitter for kPrimQuads / kPrimTriangles / kPrimTriangleStrip. These
    // were three verbatim copies of the same 30 lines, each submitting ONE
    // gsKit prim per triangle — and every such call writes its own GIF tag plus
    // a TEX0 register pair. Text is the worst case: FontRenderer draws one quad
    // per glyph, so a debug overlay or an open inventory was spending most of
    // its GS bandwidth on tags. Triangles are accumulated here and handed over
    // in one list, exactly like the terrain fast path does.
    //
    // Batching never reorders anything: it only merges consecutive triangles
    // inside a single glDrawArrays call, which already share one texture and
    // one blend state. The batch is flushed before any clamp change and before
    // returning, so nothing outlives the call.
    static GSPRIMSTQPOINT s_orthoBatch[PS2_ORTHO_BATCH_TRIS * 3] __attribute__((aligned(16)));
    int orthoBatchCount = 0;                 // triangles staged
    bool orthoClampValid = false;
    Ps2ClampSel orthoClampLast = { -1, -1, -1 };

    auto flushOrthoBatch = [&]() {
        if (orthoBatchCount <= 0) return;
        ps2_gs_queue_guard(orthoBatchCount * 3);
        gsKit_prim_list_triangle_goraud_texture_stq_3d(
            gsGlobal, tex, orthoBatchCount * 3, s_orthoBatch);
        orthoBatchCount = 0;
    };

    // Ortho triangle from three array indices. svq always succeeds here (no
    // near plane in ortho), so there is nothing to clip.
    auto emit_ortho_tri = [&](int i0, int i1, int i2) {
        float x0,y0,x1,y1,x2,y2;
        int z0, z1, z2;
        svq(i0,x0,y0,z0); svq(i1,x1,y1,z1); svq(i2,x2,y2,z2);

        unsigned char r0,g0,b0_,a0, r1,g1,b1_,a1, r2,g2,b2_,a2;
        getcol(i0,r0,g0,b0_,a0); getcol(i1,r1,g1,b1_,a1); getcol(i2,r2,g2,b2_,a2);
        if (!renderState.smoothShading) {
            r0 = r1 = r2; g0 = g1 = g2; b0_ = b1_ = b2_; a0 = a1 = a2;
        }

        if (!tex) {
            ps2_gs_queue_guard(3);
            gsKit_prim_triangle_gouraud_3d(gsGlobal,
                x0,y0,z0, x1,y1,z1, x2,y2,z2,
                mkcol(r0,g0,b0_,a0), mkcol(r1,g1,b1_,a1), mkcol(r2,g2,b2_,a2));
            return;
        }

        const float* t0 = at_(tc, i0);
        const float* t1 = at_(tc, i1);
        const float* t2 = at_(tc, i2);

        // gsKit_set_clamp writes into the DMA stream immediately, so anything
        // already staged must go out under the clamp it was built with.
        float minU, minV, maxU, maxV;
        ps2_uv_bounds3(t0[0]*texW, t0[1]*texH,
                       t1[0]*texW, t1[1]*texH,
                       t2[0]*texW, t2[1]*texH,
                       minU, minV, maxU, maxV);
        const Ps2ClampSel sel = ps2_select_clamp(texW, texH, true, false,
                                                 minU, minV, maxU, maxV);
        if (!orthoClampValid || sel != orthoClampLast) {
            flushOrthoBatch();
            ps2_apply_clamp_sel(sel.mode, sel.ufix, sel.vfix);
            orthoClampLast = sel;
            orthoClampValid = true;
        }

        if (orthoBatchCount == PS2_ORTHO_BATCH_TRIS)
            flushOrthoBatch();

        // q = 1 throughout: ortho has no perspective divide, so STQ is the raw UV.
        GSPRIMSTQPOINT* p = &s_orthoBatch[orthoBatchCount * 3];
        p[0].rgbaq = color_to_RGBAQ(PS2_TEXCOL_GUI(r0),PS2_TEXCOL_GUI(g0),PS2_TEXCOL_GUI(b0_),(u8)(a0>>1),1.0f);
        p[0].stq   = vertex_to_STQ(t0[0], t0[1]);
        p[0].xyz2  = vertex_to_XYZ2(gsGlobal, x0, y0, z0);
        p[1].rgbaq = color_to_RGBAQ(PS2_TEXCOL_GUI(r1),PS2_TEXCOL_GUI(g1),PS2_TEXCOL_GUI(b1_),(u8)(a1>>1),1.0f);
        p[1].stq   = vertex_to_STQ(t1[0], t1[1]);
        p[1].xyz2  = vertex_to_XYZ2(gsGlobal, x1, y1, z1);
        p[2].rgbaq = color_to_RGBAQ(PS2_TEXCOL_GUI(r2),PS2_TEXCOL_GUI(g2),PS2_TEXCOL_GUI(b2_),(u8)(a2>>1),1.0f);
        p[2].stq   = vertex_to_STQ(t2[0], t2[1]);
        p[2].xyz2  = vertex_to_XYZ2(gsGlobal, x2, y2, z2);
        orthoBatchCount++;
    };

    // Ortho lines. No texture and no batch: a LINE carries a different PRIM
    // than the staged triangle list, so it could not join that packet anyway,
    // and a draw call is all one mode -- a line call never has triangles
    // waiting behind it.
    auto emit_ortho_line = [&](int i0, int i1) {
        float x0, y0, x1, y1;
        int z0, z1;
        svq(i0, x0, y0, z0); svq(i1, x1, y1, z1);

        unsigned char r0,g0,b0_,a0, r1,g1,b1_,a1;
        getcol(i0, r0,g0,b0_,a0); getcol(i1, r1,g1,b1_,a1);
        if (!renderState.smoothShading) { r0 = r1; g0 = g1; b0_ = b1_; a0 = a1; }

        ps2_gs_queue_guard(2);
        gsKit_prim_line_goraud_3d(gsGlobal, x0, y0, z0, x1, y1, z1,
            mkcol(r0,g0,b0_,a0), mkcol(r1,g1,b1_,a1));
    };

    // One triangle from three array indices, whichever mode we are in. The
    // ortho/3D branch used to be written out in all three loops below.
    // Deliberately NOT used by the kPrimQuads 3D case: a quad shares two corners
    // between its triangles, and routing it through here would transform six
    // vertices where four suffice — 50% more VU0 work on the hottest path in
    // the game.
    auto emit_tri = [&](int i0, int i1, int i2) {
        if (ps2_projection_is_orthographic()) { emit_ortho_tri(i0, i1, i2); return; }
        ClipVert tri[3] = { make_cv(i0), make_cv(i1), make_cv(i2) };
        submit_clip_tri(tri);
    };

    auto emit_line = [&](int i0, int i1) {
        if (ps2_projection_is_orthographic()) { emit_ortho_line(i0, i1); return; }
        submit_clip_line(make_cv(i0), make_cv(i1));
    };

    auto emit_point = [&](int i) {
        unsigned char r, g, b, a;
        getcol(i, r, g, b, a);
        float x, y;
        int z;
        if (ps2_projection_is_orthographic()) {
            svq(i, x, y, z);
        } else {
            ClipVert cv = make_cv(i);
            const int oc = ps2_clip_outcode(cv.cx, cv.cy, cv.cz, cv.cw, gbx, gby);
            if (oc != 0) { PS2_RENDER_STAT(s_dbg_3d_clip++); return; }
            float q;
            project_cv(cv, scrW, scrH, x, y, z, q);
            if (x < 0.0f || y < 0.0f || x >= scrW || y >= scrH) {
                PS2_RENDER_STAT(s_dbg_3d_offscreen++);
                return;
            }
        }
        ps2_gs_queue_guard(1);
        gsKit_prim_point(gsGlobal, x, y, z, mkcol(r, g, b, a));
    };

    if (mode == kPrimPoints) {
        for (int i = 0; i < count; ++i)
            emit_point(first + i);
    } else if (mode == kPrimQuads) {
        const int nq = count / 4;
        for (int qi = 0; qi < nq; qi++) {
            const int b = first + qi*4;
            if (!ps2_projection_is_orthographic()) {
                // 3D: four transforms, not six — v0 and v2 are shared. Each
                // sub-triangle is still clipped independently against the near
                // plane.
                ClipVert v0=make_cv(b+0), v1=make_cv(b+1), v2=make_cv(b+2), v3=make_cv(b+3);
                ClipVert tri1[3]={v0,v1,v2}; submit_clip_tri(tri1);
                ClipVert tri2[3]={v0,v2,v3}; submit_clip_tri(tri2);
            } else {
                // Two explicit triangles, not gsKit_prim_quad_*: that helper
                // emits a TRISTRIP, so it expects the "Z" corner order
                // (TL,TR,BL,BR). kPrimQuads hands us perimeter order, and a strip
                // over perimeter corners draws (0,1,2)+(1,2,3), which leaves one
                // wedge of the rectangle unfilled and doubles another — every
                // untextured GUI rect would render bowtied.
                emit_ortho_tri(b+0, b+1, b+2);
                emit_ortho_tri(b+0, b+2, b+3);
            }
        }
    } else if (mode == kPrimTriangles) {
        const int nt = count / 3;
        for (int t = 0; t < nt; t++) {
            const int b = first + t*3;
            emit_tri(b+0, b+1, b+2);
        }
    } else if (mode == kPrimTriangleStrip) {
        for (int i = 0; i + 2 < count; i++) {
            // Odd triangles swap the first two indices to keep winding.
            const int b0 = first + (i & 1 ? i+1 : i);
            const int b1 = first + (i & 1 ? i   : i+1);
            emit_tri(b0, b1, first + i + 2);
        }
    } else if (mode == kPrimTriangleFan) {
        // The sunrise/sunset glow in RenderGlobal::renderSky is the game's only
        // fan, and it was the one primitive no PS2 path handled: the mode fell
        // through this switch and the draw silently produced nothing.
        for (int i = 1; i + 1 < count; i++)
            emit_tri(first, first + i, first + i + 1);
    } else if (mode == kPrimLines || mode == kPrimLineStrip || mode == kPrimLineLoop) {
        // The block selection box (kPrimLines + kPrimLineStrip) and the fishing
        // line (kPrimLineStrip) were in the same position as the fan above.
        // kPrimLines consumes disjoint pairs; the strip/loop forms chain. An odd
        // trailing vertex in a kPrimLines batch has no partner and is dropped,
        // which is what desktop GL does with it too.
        const int step = (mode == kPrimLines) ? 2 : 1;
        for (int i = 0; i + 1 < count; i += step)
            emit_line(first + i, first + i + 1);
        // kPrimLineLoop closes back onto the first vertex.
        if (mode == kPrimLineLoop && count > 2)
            emit_line(first + count - 1, first);
    }

    flushOrthoBatch();
}

#endif // PS2_PLATFORM
