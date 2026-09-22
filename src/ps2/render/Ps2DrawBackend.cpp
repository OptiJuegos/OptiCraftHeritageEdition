#ifdef PS2_PLATFORM

#include "platform/Log.h"
#include "ps2/render/Ps2ClipGuard.h"
#include "ps2/render/Ps2Draw2D.h"
#include "ps2/render/Ps2DrawCommon.h"
#include "ps2/render/Ps2DrawImmediate.h"
#include "ps2/render/Ps2Draw3D.h"
#include "ps2/render/Ps2GsQueue.h"
#include "ps2/render/Ps2MatrixStack.h"
#include "ps2/render/Ps2NativeDraw.h"
#include "ps2/render/Ps2ProjectionState.h"
#include "ps2/render/Ps2RenderBackend.h"
#include "ps2/render/Ps2RenderContext.h"
#include "ps2/render/Ps2RenderGsState.h"
#include "ps2/render/Ps2RenderLighting.h"
#include "ps2/render/Ps2RenderState.h"
#include "ps2/render/Ps2RenderStats.h"
#include "ps2/render/Ps2RenderTextureState.h"
#include "ps2/render/Ps2RenderTransform.h"
#include "ps2/render/Ps2RenderTypes.h"
#include "ps2/render/Ps2TerrainRenderer.h"
#include "ps2/render/Ps2Texture.h"
#include "ps2/render/Ps2TextureGs.h"
#include "ps2/render/Ps2Tuning.h"
#include "ps2/render/Ps2Viewport.h"
#include "ps2/render/Ps2Vu0Math.h"

#include <gsCore.h>
#include <gsInline.h>
#include <gsKit.h>
#include <gsMisc.h>
#include <gsPrimitive.h>
#include <gsTexture.h>
#include <libvux.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

namespace
{
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
constexpr unsigned int kFaceCCW = 0x0901;
constexpr unsigned int kFaceBack = ps2RenderValue(Ps2RenderFace::Back);
constexpr unsigned int kFogLinear = ps2RenderValue(Ps2RenderFogMode::Linear);
}

typedef float Mat4[16];

extern GSGLOBAL* gsGlobal;
static Ps2RenderContext& st = ps2_render_context();

// The GL wrap/filter enums do not survive the trip to the GS one-to-one, so
// the compatibility layer collapses each pname to the boolean the native API
// takes and lets Ps2Texture document what the hardware does with it.



// Near-Z override is transform state, exposed here for legacy callers.
void ps2glForceNearZ(bool on) { ps2_transform_set_force_near_z(on); }


// GS draw-state programming is owned by Ps2RenderGsState.

// Shared draw state/texture resolution and the optimized VU0 3D entry point
// live in Ps2DrawCommon.

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
    // VU1 supports linear and exponential fog. EXP2 still uses VU0.
    if (st.fog && st.fogMode != kFogLinear &&
        st.fogMode != ps2RenderValue(Ps2RenderFogMode::Exp))
        return false;

    // Apply TEST/Z/FRAME/ALPHA state and resolve deferred texture updates while
    // we still own Path3. Reserve queue slack before any of those register
    // writes: terrain pass setup is not reached through the normal draw guard.
    // ps2_render_acquire_path1() drains this queue before VIF1 emits Path1 data.
    ps2_gs_queue_guard(0);
    ps2_gs_state_apply_draw_state();
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
    const Ps2TextureSampler sampler = ps2_texture_sampler_registers(texture);
    out.tex1 = sampler.tex1;
    out.miptbp1 = sampler.miptbp1;
    out.mipmapped = sampler.mipmapped;
    out.texa = GS_SETREG_TEXA(0x00, 1, 0x80);
    out.alpha = ps2_gs_state_blend_alpha_valid()
        ? ps2_gs_state_blend_alpha_reg()
        : GS_SETREG_ALPHA(0, 1, 0, 1, 0);

    out.textureWidth = texture->Width;
    out.textureHeight = texture->Height;
    out.primContext = gsGlobal->PrimContext;
    const GSTEST* test = gsGlobal->Test;
    out.test = GS_SETREG_TEST(test->ATE, test->ATST, test->AREF, test->AFAIL,
                              test->DATE, test->DATM, test->ZTE, test->ZTST);
    out.zbuf = GS_SETREG_ZBUF(gsGlobal->ZBuffer / 8192, gsGlobal->PSMZ, ps2_gs_state_zmsk());
    out.primAlphaEnable = gsGlobal->PrimAlphaEnable;
    out.primAAEnable = gsGlobal->PrimAAEnable;
    out.offsetX = gsGlobal->OffsetX;
    out.offsetY = gsGlobal->OffsetY;
    out.viewW = ps2_viewport_width();
    out.viewH = ps2_viewport_height();
    out.depthQScale = ps2_transform_depth_map().qScale;
    out.depthQBias = ps2_transform_depth_map().qBias + ps2_gs_state_polygon_depth_bias();
    out.forceNearZ = ps2_transform_force_near_z_enabled();
    out.render = ps2_draw_capture_render_state(true, true);
    return true;
}

void ps2_render_invalidate_path1_state()
{
    ps2_gs_state_invalidate_path1();
    ps2_texture_state_invalidate();
}

void ps2_render_invalidate_framebuffer_state()
{
    ps2_gs_state_invalidate_framebuffer();
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

    const Ps2LightingArrayState oldLightingArray = ps2_lighting_array_state();

    const bool normalArrayEnabled = mesh.hasNormals && mesh.normals != nullptr;
    ps2_lighting_set_array(normalArrayEnabled ? mesh.normals : nullptr,
                           normalArrayEnabled ? (int)mesh.normalStride : 0,
                           normalArrayEnabled && mesh.normalFloat ? kTypeFloat : kTypeByte,
                           normalArrayEnabled);

    gsGlobal->ZBuffering = GS_SETTING_ON;
    ps2_gs_queue_guard_prim(mode, mesh.count);
    ps2_gs_state_apply_draw_state();

    Ps2Draw2DState drawState;
    drawState.gsGlobal = gsGlobal;
    drawState.texture = ps2_draw_resolve_texture(mesh.texCoordEnabled && mesh.texCoords != nullptr);
	drawState.modelView = ps2_matrix_model_view();
    drawState.render = ps2_draw_capture_render_state(false, false);
    drawState.left = ps2_projection_state().left;
    drawState.right = ps2_projection_state().right;
    drawState.bottom = ps2_projection_state().bottom;
    drawState.top = ps2_projection_state().top;
    drawState.nearPlane = ps2_projection_state().nearPlane;
    drawState.farPlane = ps2_projection_state().farPlane;
    drawState.viewW = ps2_viewport_width();
    drawState.viewH = ps2_viewport_height();
    drawState.depthMax = PS2_GS_Z_MAX;
    drawState.depthTest = ps2_gs_state_depth_test_enabled();
    drawState.queueGuard = ps2_gs_queue_guard;
    drawState.applyClampSel = ps2_texture_state_apply_clamp_sel;
#ifdef PS2_RENDER_STATS
    drawState.debugSprites = &ps2_render_stats().orthoSprites;
    drawState.debugSpriteFallbacks = &ps2_render_stats().orthoSpriteFallbacks;
    drawState.debugDepthTested = &ps2_render_stats().orthoZTested;
    drawState.debugDepthPinned = &ps2_render_stats().orthoZPinned;
    drawState.debugDepthLow = &ps2_render_stats().orthoZLow;
    drawState.debugDepthHigh = &ps2_render_stats().orthoZHigh;
    drawState.debugEyeZLow = &ps2_render_stats().orthoEyeZLow;
    drawState.debugEyeZHigh = &ps2_render_stats().orthoEyeZHigh;
    drawState.debugNear = &ps2_render_stats().orthoNear;
    drawState.debugFar = &ps2_render_stats().orthoFar;
#endif

    const bool consumed = ps2_draw_2d_mesh(mesh, drawState);
#if MC_LOG_LEVEL > 2
    if (consumed)
    {
        ++ps2_render_stats().profile3DrawCalls;
        ps2_render_stats().profile3Vertices += mesh.count;
    }
#endif

    ps2_lighting_restore_array(oldLightingArray);
    return consumed;
}

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
    if (ps2_lighting_enabled() && (!allowLighting || !mesh.hasNormals || mesh.normals == nullptr))
        return false;

    Ps2BackendArrayState old = {
        st.vp, st.vstride, st.vsize, st.tp, st.tstride,
        st.cp, st.cstride, st.csize, st.cfloat,
        st.ven, st.ten, st.cen
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
    const Ps2LightingArrayState oldLightingArray = ps2_lighting_array_state();
    const bool normalArrayEnabled = allowLighting && mesh.hasNormals && mesh.normals != nullptr;
    ps2_lighting_set_array(normalArrayEnabled ? mesh.normals : nullptr,
                           normalArrayEnabled ? mesh.normalStride : 0,
                           normalArrayEnabled && mesh.normalFloat ? kTypeFloat : kTypeByte,
                           normalArrayEnabled);

    ps2_draw_immediate_arrays(mode, mesh.first, mesh.count, preparedMvp);

    st.vp = old.vp; st.vstride = old.vstride; st.vsize = old.vsize;
    st.tp = old.tp; st.tstride = old.tstride;
    st.cp = old.cp; st.cstride = old.cstride; st.csize = old.csize; st.cfloat = old.cfloat;
    st.ven = old.ven; st.ten = old.ten; st.cen = old.cen;
    ps2_lighting_restore_array(oldLightingArray);
    return true;
}

static bool ps2_native_mesh_state_supported(const Ps2NativeMeshView& mesh, bool allowLighting) {
    const unsigned int mode = (unsigned int)mesh.drawMode;
    if (!gsGlobal || !mesh.vertices || mesh.count < 1)
        return false;
	if (!ps2_draw_3d_enabled() || (mode != kPrimTriangles && mode != kPrimQuads)
            || ps2_projection_is_orthographic())
        return false;
    // Prepared terrain contexts carry only the final MVP, not the modelview
    // needed to transform normals. Direct model draws may use the live
    // modelview/light state when an explicit normal stream is available.
    if (ps2_lighting_enabled() && (!allowLighting || !mesh.hasNormals || mesh.normals == nullptr))
        return false;
    return true;
}

bool ps2_native_prepare_frame_context(Ps2NativeFrameContext& out) {
    out.valid = false;
    out.viewW = 0.0f;
    out.viewH = 0.0f;
	if (!gsGlobal || ps2_projection_is_orthographic() || !ps2_draw_3d_enabled())
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
    //
    // Both matrices declare 16-byte alignment, so the COP2 form is the normal
    // path; the guard only has to catch a caller that placed a context in
    // storage the declaration cannot reach, exactly as ps2_matrix_multiply
    // guards multiplyVu0.
    if (ps2_vu0_is_aligned16(out.mvp) && ps2_vu0_is_aligned16(frame.mvp)) {
        VU_VECTOR translate __attribute__((aligned(16)));
        translate.x = tx;
        translate.y = ty;
        translate.z = tz;
        translate.w = 0.0f;
        ps2_vu0_mvp_translate(out.mvp, frame.mvp, &translate);
        out.valid = true;
        return true;
    }

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
    PS2_RENDER_STAT(ps2_render_stats().nativeAttempts++);
    if (!context.valid || !ps2_native_mesh_state_supported(mesh, allowLighting))
        return false;

    const Ps2LightingArrayState oldLightingArray = ps2_lighting_array_state();
    const bool useNativeNormals = allowLighting && ps2_lighting_enabled() && mesh.hasNormals;
    if (useNativeNormals) {
        ps2_lighting_set_array(mesh.normals, (int)mesh.normalStride,
                               mesh.normalFloat ? kTypeFloat : kTypeByte, true);
    }

    const unsigned int mode = (unsigned int)mesh.drawMode;
    gsGlobal->ZBuffering = GS_SETTING_ON;
    PS2_RENDER_STAT(ps2_render_stats().draw3dCalls++);
    // Barrier: staged 2D sprites were built under the state and packet
    // order that precede this draw, so they leave before it does.
    ps2_draw_2d_flush_pending();
    ps2_gs_queue_guard_prim(mode, mesh.count);
    ps2_gs_state_apply_draw_state();

    GSTEXTURE* tex = ps2_draw_resolve_texture(mesh.texCoordEnabled && mesh.texCoords != nullptr);
	const bool consumed = ps2_try_draw_3d(mode, mesh.first, mesh.count, tex, context.mvp,
												mesh.packedTerrain,
                                                    mesh.clampRuns, mesh.clampRunCount,
                                                    mesh.slices, mesh.sliceCount,
                                                    mesh.vertices, mesh.vertexStride, mesh.vertexSize,
                                                    mesh.texCoords, mesh.texCoordStride, mesh.texCoordEnabled,
                                                    mesh.colors, mesh.colorStride, mesh.colorSize,
                                                    mesh.colorEnabled, mesh.colorFloat,
                                                    context.fullyInside);

    if (useNativeNormals) {
        ps2_lighting_restore_array(oldLightingArray);
    }

    PS2_RENDER_STAT(if (consumed) ps2_render_stats().nativeHits++);
#if MC_LOG_LEVEL > 2
    if (consumed)
    {
        ++ps2_render_stats().profile3DrawCalls;
        ps2_render_stats().profile3Vertices += mesh.count;
    }
#endif
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
        PS2_RENDER_STAT(ps2_render_stats().nativeAttempts++);
        if (ps2_native_draw_ortho_mesh(mesh)) {
            PS2_RENDER_STAT(ps2_render_stats().nativeHits++);
            return true;
        }
        const bool genericConsumed = ps2_backend_draw_mesh_generic(mesh, nullptr, true);
#if MC_LOG_LEVEL > 2
        if (genericConsumed)
        {
            ++ps2_render_stats().profile3DrawCalls;
            ps2_render_stats().profile3Vertices += mesh.count;
        }
#endif
        return genericConsumed;
    }

    Ps2NativeDrawContext context;
    context.valid = true;
	context.fullyInside = ps2_draw_3d_fully_inside();
	ps2_matrix_multiply(context.mvp, ps2_matrix_projection(), ps2_matrix_model_view());
    if (ps2_native_draw_mesh_with_context(mesh, context, true))
        return true;
    return ps2_backend_draw_mesh_generic(mesh, nullptr, true);
}

#endif // PS2_PLATFORM
