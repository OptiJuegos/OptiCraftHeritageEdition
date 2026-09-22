#pragma once

#ifdef PS2_PLATFORM

#include <gsKit.h>

void ps2_gs_state_set_ztst(u8 ztst);
void ps2_gs_state_set_zmsk(u8 zmsk);
u8 ps2_gs_state_zmsk();

void ps2_gs_state_apply_texa();
void ps2_gs_state_apply_fog_color(float red, float green, float blue);
void ps2_gs_state_apply_frame_mask(u32 mask);

void ps2_gs_state_set_blend_alpha(u64 alphaReg);
void ps2_gs_state_invalidate_blend_alpha();
bool ps2_gs_state_blend_alpha_valid();
u64 ps2_gs_state_blend_alpha_reg();

void ps2_gs_state_apply_alpha_test(bool enabled, u8 atst, u8 aref);
void ps2_gs_state_invalidate_alpha_test();

void ps2_gs_state_record_prim_alpha(bool enabled);
void ps2_gs_state_invalidate_prim_alpha();

void ps2_gs_state_invalidate_depth();
void ps2_gs_state_invalidate_after_clear();
void ps2_gs_state_invalidate_path1();
void ps2_gs_state_invalidate_framebuffer();

#endif // PS2_PLATFORM

#ifdef PS2_PLATFORM

// High-level fixed-function state. These functions own the GL-like state that
// is translated into GS registers; Ps2RenderBackend only routes public API calls.
void ps2_gs_state_set_blend_func(unsigned int source, unsigned int destination);
void ps2_gs_state_apply_blend();
void ps2_gs_state_force_fix_blend(bool enabled, u8 fix);

void ps2_gs_state_set_depth_test(bool enabled);
bool ps2_gs_state_depth_test_enabled();
void ps2_gs_state_set_depth_mask(bool enabled);
bool ps2_gs_state_depth_mask_enabled();
void ps2_gs_state_set_depth_func(unsigned int compare);
unsigned int ps2_gs_state_depth_func();
void ps2_gs_state_apply_depth();

void ps2_gs_state_set_polygon_offset_enabled(bool enabled);
void ps2_gs_state_set_polygon_offset_units(float units);
float ps2_gs_state_polygon_depth_bias();
int ps2_gs_state_apply_polygon_depth_bias(int depth);

void ps2_gs_state_set_color_mask(bool red, bool green, bool blue, bool alpha);
void ps2_gs_state_apply_color_mask();

void ps2_gs_state_set_shade_model(unsigned int model);
bool ps2_gs_state_smooth_shading();

void ps2_gs_state_apply_current_alpha_test();
void ps2_gs_state_apply_draw_state();
void ps2_gs_state_clear_depth_only();

void ps2_gs_state_begin_terrain_pass(bool translucent);
void ps2_gs_state_end_terrain_pass(bool translucent);

#endif // PS2_PLATFORM
