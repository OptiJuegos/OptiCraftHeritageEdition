#pragma once

#ifdef PS2_PLATFORM

#include "ps2/render/Ps2RenderTypes.h"

// Native fixed-function entry points for the PS2 backend.
//
// This is the PS2 half of platform/RenderAPI.h. Until now only the Wii had one:
// RenderAPI dispatches directly to the PS2 fixed-function backend; no OpenGL
// compatibility entry points are involved on this platform. The backend owns
// the render state and translates it into GS/VU operations.
//
// Selectors are the uint32_t values of Ps2RenderTypes.h, unwrapped with
// ps2RenderValue().

void ps2_render_enable(unsigned int capability);
void ps2_render_disable(unsigned int capability);

void ps2_render_blend_func(unsigned int source, unsigned int destination);
void ps2_render_alpha_func(unsigned int compare, float reference);
void ps2_render_depth_func(unsigned int compare);
void ps2_render_depth_mask(bool enabled);
void ps2_render_color_mask(bool red, bool green, bool blue, bool alpha);
void ps2_render_cull_face(unsigned int face);
void ps2_render_shade_model(unsigned int model);

void ps2_render_color4f(float r, float g, float b, float a);
void ps2_render_normal3f(float x, float y, float z);

void ps2_render_fogf(unsigned int parameter, float value);
void ps2_render_fog_color(const float* values);
// Linear is the only mode the backend implements; see the definition.
void ps2_render_fog_mode(unsigned int mode);

// lightIndex is 0 or 1. Higher lights are ignored -- the PS2 lighting path
// carries two directional lights, matching RenderHelper's setup.
void ps2_render_lightfv(int lightIndex, unsigned int parameter, const float* values);
void ps2_render_light_model_ambient(const float* values);
void ps2_render_color_material(unsigned int face, unsigned int mode);
void ps2_render_set_lightmap_enabled(bool enabled);
void ps2_render_set_lightmap_coord(float u, float v);
void ps2_render_set_lightmap_colors(const unsigned int* colors, int count);

void ps2_render_clear(unsigned int mask);
void ps2_render_clear_color(float r, float g, float b, float a);
void ps2_render_clear_depth(float depth);

void ps2_render_viewport(int x, int y, int width, int height);
void ps2_render_get_viewport(int* values);
void ps2_render_polygon_offset(float factor, float units);
void ps2_render_line_width(float width);
void ps2_render_fog_hint(bool nicest);

void ps2_render_matrix_mode(unsigned int mode);
void ps2_render_load_identity();
void ps2_render_push_matrix();
void ps2_render_pop_matrix();
void ps2_render_translate(float x, float y, float z);
void ps2_render_rotate(float angle, float x, float y, float z);
void ps2_render_scale(float x, float y, float z);
// Float, not double, all the way in: the matrix these build is Mat4 (float) and
// the GS consumes 32-bit floats, so a double parameter only buys a software
// narrowing on a CPU with no double FPU. The shared platform/RenderAPI.h keeps
// its double signature -- the GL backend forwards straight into glFrustum -- and
// RenderAPI_GS_PS2.cpp narrows once at the boundary.
void ps2_render_frustum(float left, float right, float bottom, float top,
                        float nearValue, float farValue);
void ps2_render_ortho(float left, float right, float bottom, float top,
                      float nearValue, float farValue);

void ps2_render_get_matrix(unsigned int query, float* values);
const unsigned char* ps2_render_get_string();
unsigned int ps2_render_get_error();

#endif // PS2_PLATFORM
