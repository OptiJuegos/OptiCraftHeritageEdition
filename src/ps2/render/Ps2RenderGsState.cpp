#ifdef PS2_PLATFORM

#include "ps2/render/Ps2RenderGsState.h"

#include "ps2/render/Ps2Draw2D.h"
#include "ps2/render/Ps2RenderBackend.h"

#include <gsCore.h>
#include <gsInline.h>
#include <gsMisc.h>
#include <gsPrimitive.h>

extern GSGLOBAL* gsGlobal;

namespace
{
u8 s_gsZtst = 0xFF;
u8 s_gsZmsk = 0xFF;
bool s_gsFrameMaskValid = false;
u32 s_gsFrameMask = 0;
u32 s_gsFrameFbp = 0;

bool s_primAlphaEnableValid = false;
bool s_primAlphaEnable = false;
bool s_blendAlphaValid = false;
u64 s_blendAlphaReg = 0;
bool s_alphaTestValid = false;
bool s_alphaTestOn = false;
u8 s_alphaTestAtst = 0;
u8 s_alphaTestAref = 0;
bool s_texaValid = false;
bool s_fogColorValid = false;
u64 s_fogColorReg = 0;

void ps2_gs_write_reg(u64 data, u64 address)
{
    if (!gsGlobal || gsGlobal->CurQueue == nullptr)
        return;

    // Barrier for the deferred 2D batch: anything already staged was built
    // against the register value this call is about to replace, so it has to
    // reach the GS first. Callers reach here only after their own change
    // detection, so a batch survives a redundant state apply.
    ps2_draw_2d_flush_pending();

    u64* packet = (u64*)gsKit_heap_alloc(gsGlobal, 1, 16, GIF_AD);
    if (!packet)
        return;

    *packet++ = GIF_TAG_AD(1);
    *packet++ = GIF_AD;
    *packet++ = data;
    *packet++ = address;
}
} // namespace

void ps2_gs_state_set_ztst(u8 ztst)
{
    if (!gsGlobal || !gsGlobal->Test || s_gsZtst == ztst)
        return;

    gsGlobal->Test->ZTST = ztst;
    const GSTEST* test = gsGlobal->Test;
    ps2_gs_write_reg(GS_SETREG_TEST(test->ATE, test->ATST, test->AREF, test->AFAIL,
                                    test->DATE, test->DATM, 1, ztst),
                     GS_TEST_1 + gsGlobal->PrimContext);
    s_gsZtst = ztst;
}

void ps2_gs_state_set_zmsk(u8 zmsk)
{
    if (!gsGlobal || s_gsZmsk == zmsk)
        return;

    ps2_gs_write_reg(GS_SETREG_ZBUF(gsGlobal->ZBuffer / 8192, gsGlobal->PSMZ, zmsk),
                     GS_ZBUF_1 + gsGlobal->PrimContext);
    s_gsZmsk = zmsk;
}

u8 ps2_gs_state_zmsk()
{
    return s_gsZmsk;
}

void ps2_gs_state_apply_texa()
{
    if (!gsGlobal || s_texaValid)
        return;

    ps2_gs_write_reg(GS_SETREG_TEXA(0x00, 1, 0x80), GS_TEXA);
    s_texaValid = true;
}

void ps2_gs_state_apply_fog_color(float red, float green, float blue)
{
    if (!gsGlobal)
        return;

    auto toByte = [](float value) -> u8 {
        if (value <= 0.0f) return 0;
        if (value >= 1.0f) return 255;
        return (u8)(value * 255.0f + 0.5f);
    };

    const u64 fogColor = (u64)toByte(red) |
        ((u64)toByte(green) << 8) |
        ((u64)toByte(blue) << 16);
    if (s_fogColorValid && s_fogColorReg == fogColor)
        return;

    ps2_gs_write_reg(fogColor, GS_FOGCOL);
    s_fogColorValid = true;
    s_fogColorReg = fogColor;
}

void ps2_gs_state_apply_frame_mask(u32 mask)
{
    if (!gsGlobal)
        return;

    const u32 drawBuffer = gsGlobal->ScreenBuffer[gsGlobal->ActiveBuffer & 1];
    const u32 fbp = drawBuffer / 8192;
    if (s_gsFrameMaskValid && s_gsFrameMask == mask && s_gsFrameFbp == fbp)
        return;

    const u64 frame = GS_SETREG_FRAME(fbp, gsGlobal->Width / 64, gsGlobal->PSM, mask);
    ps2_gs_write_reg(frame, GS_FRAME_1);
    ps2_gs_write_reg(frame, GS_FRAME_2);

    s_gsFrameMaskValid = true;
    s_gsFrameMask = mask;
    s_gsFrameFbp = fbp;
}

void ps2_gs_state_set_blend_alpha(u64 alphaReg)
{
    if (!gsGlobal)
        return;
    if (s_blendAlphaValid && s_blendAlphaReg == alphaReg)
        return;

    ps2_draw_2d_flush_pending();
    gsKit_set_primalpha(gsGlobal, alphaReg, 0);
    s_blendAlphaValid = true;
    s_blendAlphaReg = alphaReg;
}

void ps2_gs_state_invalidate_blend_alpha()
{
    s_blendAlphaValid = false;
}

bool ps2_gs_state_blend_alpha_valid()
{
    return s_blendAlphaValid;
}

u64 ps2_gs_state_blend_alpha_reg()
{
    return s_blendAlphaReg;
}

void ps2_gs_state_apply_alpha_test(bool enabled, u8 atst, u8 aref)
{
    if (!gsGlobal || !gsGlobal->Test)
        return;
    if (s_alphaTestValid && s_alphaTestOn == enabled &&
        s_alphaTestAtst == atst && s_alphaTestAref == aref)
        return;

    ps2_draw_2d_flush_pending();
    if (enabled)
    {
        gsGlobal->Test->ATST = atst;
        gsGlobal->Test->AREF = aref;
        gsGlobal->Test->AFAIL = 0;
        gsKit_set_test(gsGlobal, GS_ATEST_ON);
    }
    else
    {
        gsKit_set_test(gsGlobal, GS_ATEST_OFF);
    }

    s_alphaTestValid = true;
    s_alphaTestOn = enabled;
    s_alphaTestAtst = atst;
    s_alphaTestAref = aref;
}

void ps2_gs_state_invalidate_alpha_test()
{
    s_alphaTestValid = false;
}

void ps2_gs_state_record_prim_alpha(bool enabled)
{
    s_primAlphaEnableValid = true;
    s_primAlphaEnable = enabled;
}

void ps2_gs_state_invalidate_prim_alpha()
{
    s_primAlphaEnableValid = false;
}

void ps2_gs_state_invalidate_depth()
{
    s_gsZtst = 0xFF;
    s_gsZmsk = 0xFF;
}

void ps2_gs_state_invalidate_after_clear()
{
    ps2_gs_state_invalidate_depth();
    s_gsFrameMaskValid = false;
    s_alphaTestValid = false;
    s_blendAlphaValid = false;
    s_primAlphaEnableValid = false;
    s_texaValid = false;
    s_fogColorValid = false;
}

void ps2_gs_state_invalidate_path1()
{
    ps2_gs_state_invalidate_after_clear();
}

void ps2_gs_state_invalidate_framebuffer()
{
    s_gsFrameMaskValid = false;
}

#endif // PS2_PLATFORM

#ifdef PS2_PLATFORM

#include "platform/Log.h"
#include "ps2/render/Ps2GsQueue.h"
#include "ps2/render/Ps2RenderContext.h"
#include "ps2/render/Ps2RenderStats.h"
#include "ps2/render/Ps2RenderTypes.h"
#include "ps2/render/Ps2Tuning.h"

namespace
{
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
constexpr unsigned int kShadeSmooth = ps2RenderValue(Ps2RenderShadeModel::Smooth);

unsigned int s_blendSrc = kBlendSrcAlpha;
unsigned int s_blendDst = kBlendOneMinusSrcAlpha;
unsigned int s_blendEquationRGB = kBlendEquationAdd;
bool s_depthTestEnabled = true;
bool s_depthMaskEnabled = true;
unsigned int s_depthFunc = kCompareLessEqual;
unsigned int s_depthFuncBeforeTranslucent = kCompareLessEqual;
bool s_polygonOffsetFill = false;
float s_polygonOffsetUnits = 0.0f;
bool s_translucentDepthFuncOverridden = false;
bool s_colorMaskR = true;
bool s_colorMaskG = true;
bool s_colorMaskB = true;
bool s_colorMaskA = true;
unsigned int s_shadeModel = kShadeSmooth;
bool s_blendFixForced = false;
u8 s_blendFixValue = 0x80;

float ps2_polygon_depth_bias_gs()
{
    if (!s_polygonOffsetFill || s_polygonOffsetUnits == 0.0f)
        return 0.0f;
    return -s_polygonOffsetUnits * 2.0f;
}

u32 ps2_color_write_mask()
{
    if (!gsGlobal)
        return 0;

    u32 mask = 0;
    if (gsGlobal->PSM == GS_PSM_CT16 || gsGlobal->PSM == GS_PSM_CT16S)
    {
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

u8 ps2_compare_to_atst(unsigned int compare)
{
    switch (compare)
    {
        case kCompareNever: return 0;
        case kCompareLess: return 2;
        case kCompareLessEqual: return 3;
        case kCompareEqual: return 4;
        case kCompareGreaterEqual: return 5;
        case kCompareGreater: return 6;
        case kCompareNotEqual: return 7;
        case kCompareAlways:
        default: return 1;
    }
}
} // namespace

void ps2_gs_state_set_blend_func(unsigned int source, unsigned int destination)
{
    s_blendSrc = source;
    s_blendDst = destination;
    if (ps2_render_context().blend)
        ps2_gs_state_apply_blend();
}

void ps2_gs_state_apply_blend()
{
    if (!gsGlobal)
        return;

    if (s_blendFixForced)
    {
        ps2_gs_state_set_blend_alpha(GS_SETREG_ALPHA(0, 1, 2, 1, s_blendFixValue));
        return;
    }

    u64 alphaReg;
    if (s_blendEquationRGB == kBlendEquationReverseSubtract)
        alphaReg = GS_SETREG_ALPHA(1, 0, 2, 2, 0x80);
    else if (s_blendEquationRGB == kBlendEquationSubtract)
        alphaReg = GS_SETREG_ALPHA(0, 1, 2, 2, 0x80);
    else if (s_blendSrc == kBlendSrcAlpha && s_blendDst == kBlendOneMinusSrcAlpha)
        alphaReg = GS_SETREG_ALPHA(0, 1, 0, 1, 0);
    else if (s_blendSrc == kBlendOne && s_blendDst == kBlendOneMinusSrcAlpha)
        alphaReg = GS_SETREG_ALPHA(0, 1, 0, 1, 0);
    else if (s_blendSrc == kBlendSrcAlpha && s_blendDst == kBlendOne)
        alphaReg = GS_SETREG_ALPHA(0, 2, 0, 1, 0);
    else if (s_blendSrc == kBlendOne && s_blendDst == kBlendOne)
        alphaReg = GS_SETREG_ALPHA(0, 2, 2, 1, 0x80);
    else if (s_blendSrc == kBlendDstColor && s_blendDst == kBlendSrcColor)
        alphaReg = GS_SETREG_ALPHA(1, 0, 2, 2, 0x40);
    else if (s_blendSrc == kBlendZero && s_blendDst == kBlendOneMinusSrcColor)
        alphaReg = GS_SETREG_ALPHA(1, 0, 2, 2, 0x80);
    else if (s_blendSrc == kBlendOneMinusDstColor && s_blendDst == kBlendOneMinusSrcColor)
        alphaReg = GS_SETREG_ALPHA(0, 1, 0, 1, 0);
    else
        alphaReg = GS_SETREG_ALPHA(0, 1, 0, 1, 0);

    ps2_gs_state_set_blend_alpha(alphaReg);
}

void ps2_gs_state_force_fix_blend(bool enabled, u8 fix)
{
    s_blendFixForced = enabled;
    s_blendFixValue = fix;
    ps2_gs_state_invalidate_blend_alpha();
    if (ps2_render_context().blend)
        ps2_gs_state_apply_blend();
}

void ps2_gs_state_set_depth_test(bool enabled) { s_depthTestEnabled = enabled; }
bool ps2_gs_state_depth_test_enabled() { return s_depthTestEnabled; }
void ps2_gs_state_set_depth_mask(bool enabled) { s_depthMaskEnabled = enabled; }
bool ps2_gs_state_depth_mask_enabled() { return s_depthMaskEnabled; }
void ps2_gs_state_set_depth_func(unsigned int compare) { s_depthFunc = compare; }
unsigned int ps2_gs_state_depth_func() { return s_depthFunc; }
void ps2_gs_state_set_polygon_offset_enabled(bool enabled) { s_polygonOffsetFill = enabled; }
void ps2_gs_state_set_polygon_offset_units(float units) { s_polygonOffsetUnits = units; }

float ps2_gs_state_polygon_depth_bias()
{
    return ps2_polygon_depth_bias_gs();
}

int ps2_gs_state_apply_polygon_depth_bias(int depth)
{
    const float bias = ps2_polygon_depth_bias_gs();
    if (bias == 0.0f)
        return depth;
    int adjusted = depth + static_cast<int>(bias);
    if (adjusted < 0) adjusted = 0;
    if (adjusted > PS2_GS_Z_MAX) adjusted = PS2_GS_Z_MAX;
    return adjusted;
}

void ps2_gs_state_set_color_mask(bool red, bool green, bool blue, bool alpha)
{
    s_colorMaskR = red;
    s_colorMaskG = green;
    s_colorMaskB = blue;
    s_colorMaskA = alpha;
}

void ps2_gs_state_apply_color_mask()
{
    if (gsGlobal)
        ps2_gs_state_apply_frame_mask(ps2_color_write_mask());
}

void ps2_gs_state_set_shade_model(unsigned int model)
{
    if (model == ps2RenderValue(Ps2RenderShadeModel::Flat) || model == kShadeSmooth)
        s_shadeModel = model;
}

bool ps2_gs_state_smooth_shading() { return s_shadeModel == kShadeSmooth; }

void ps2_gs_state_apply_current_alpha_test()
{
    const Ps2RenderContext& context = ps2_render_context();
    if (!gsGlobal || !gsGlobal->Test)
        return;
    ps2_gs_state_apply_alpha_test(context.alphaTest,
                                  ps2_compare_to_atst(context.alphaFunc),
                                  context.alphaRef);
}

void ps2_gs_state_apply_depth()
{
    u8 ztst;
    u8 zmsk;
    if (s_depthTestEnabled)
    {
        static unsigned int warnedUnsupported = 0;
        switch (s_depthFunc)
        {
            case kCompareAlways: ztst = 1; break;
            case kCompareNever: ztst = 0; break;
            case kCompareLess: ztst = 3; break;
            case kCompareLessEqual: ztst = 2; break;
            case kCompareEqual:
                ztst = 2;
                if ((warnedUnsupported & 1u) == 0)
                {
                    warnedUnsupported |= 1u;
                    MC_LOG_WARN("render", "[PS2] depth EQUAL approximated with GEQUAL\n");
                }
                break;
            case kCompareGreater:
            case kCompareGreaterEqual:
            case kCompareNotEqual:
                ztst = 1;
                if ((warnedUnsupported & 2u) == 0)
                {
                    warnedUnsupported |= 2u;
                    MC_LOG_WARN("render", "[PS2] unsupported reverse depth compare approximated with ALWAYS\n");
                }
                break;
            default: ztst = 2; break;
        }
        zmsk = s_depthMaskEnabled ? 0 : 1;
    }
    else
    {
        ztst = 1;
        zmsk = 1;
    }

    ps2_gs_state_set_ztst(ztst);
    ps2_gs_state_set_zmsk(zmsk);
    PS2_DEPTH_STAT(zmsk == 0 ? ps2_render_stats().zWriteOn++ : ps2_render_stats().zWriteOff++);
}

void ps2_gs_state_apply_draw_state()
{
    Ps2RenderContext& context = ps2_render_context();
    ps2_gs_state_apply_depth();
    ps2_gs_state_apply_color_mask();
    ps2_gs_state_apply_texa();

    const bool wantAlpha = context.blend || context.alphaTest;
    gsGlobal->PrimAlphaEnable = wantAlpha ? GS_SETTING_ON : GS_SETTING_OFF;
    ps2_gs_state_record_prim_alpha(wantAlpha);
    if (context.blend)
    {
        ps2_gs_state_invalidate_blend_alpha();
        ps2_gs_state_apply_blend();
    }
    else if (context.alphaTest)
    {
        ps2_gs_state_invalidate_blend_alpha();
        ps2_gs_state_set_blend_alpha(GS_SETREG_ALPHA(0, 1, 0, 1, 0));
    }

    ps2_gs_state_apply_current_alpha_test();
}

void ps2_gs_state_clear_depth_only()
{
    if (!gsGlobal || !gsGlobal->Test)
        return;

    ps2_draw_2d_flush_pending();
    ps2_gs_queue_guard(0);
    ps2_gs_state_set_zmsk(0);
    ps2_gs_state_set_ztst(1);
    gsKit_set_test(gsGlobal, GS_ATEST_OFF);
    ps2_gs_state_invalidate_alpha_test();

    // FBMSK blocks framebuffer writes while the sprite updates only Z.
    // Restore the logical color mask immediately so following queued draws use
    // the same state they had before this depth-only clear.
    ps2_gs_state_apply_frame_mask(0xFFFFFFFFu);
    gsGlobal->PrimAlphaEnable = GS_SETTING_OFF;
    ps2_gs_state_invalidate_prim_alpha();
    ps2_gs_state_invalidate_blend_alpha();
    gsKit_prim_sprite(gsGlobal, 0.0f, 0.0f,
                      static_cast<float>(gsGlobal->Width), static_cast<float>(gsGlobal->Height),
                      0, GS_SETREG_RGBAQ(0, 0, 0, 0, 0));
    ps2_gs_state_apply_color_mask();
}

void ps2_gs_state_begin_terrain_pass(bool translucent)
{
    Ps2RenderContext& context = ps2_render_context();
    context.terrainTranslucent = translucent;
    if (translucent)
    {
        s_blendSrc = kBlendSrcAlpha;
        s_blendDst = kBlendOneMinusSrcAlpha;
        context.blend = true;
        context.cullFace = false;
        s_depthMaskEnabled = false;
        if (!s_translucentDepthFuncOverridden)
        {
            s_depthFuncBeforeTranslucent = s_depthFunc;
            s_translucentDepthFuncOverridden = true;
        }
        s_depthFunc = kCompareLess;
        // Use the material/source alpha for translucent terrain. Fog color is
        // handled per vertex in Ps2Vu0Draw3D; keeping alpha independent of fog
        // prevents water opacity from changing as the camera moves through it.
        s_blendFixForced = false;
        s_blendFixValue = 0x80;
        ps2_gs_state_invalidate_blend_alpha();
        ps2_gs_state_apply_blend();
        return;
    }

    context.blend = false;
    context.cullFace = true;
    s_depthMaskEnabled = true;
    s_blendFixForced = false;
    s_blendFixValue = 0x80;
}

void ps2_gs_state_end_terrain_pass(bool translucent)
{
    if (!translucent)
        return;

    Ps2RenderContext& context = ps2_render_context();
    context.terrainTranslucent = false;
    s_blendFixForced = false;
    s_blendFixValue = 0x80;
    ps2_gs_state_invalidate_blend_alpha();
    s_depthMaskEnabled = true;
    if (s_translucentDepthFuncOverridden)
    {
        s_depthFunc = s_depthFuncBeforeTranslucent;
        s_translucentDepthFuncOverridden = false;
    }
    context.cullFace = true;
    context.blend = false;
}

#endif // PS2_PLATFORM
