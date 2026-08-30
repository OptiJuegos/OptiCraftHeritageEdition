#pragma once

#ifdef __cplusplus
extern "C" {
#endif

enum { WII_NATIVE_MAX_LIGHTS = 2 };

typedef struct WiiNativeLightSnapshot {
    float position[4];
    float ambient[4];
    float diffuse[4];
    float specular[4];
    int enabled;
} WiiNativeLightSnapshot;

typedef struct WiiNativeRenderStateSnapshot {
    float modelview[3][4];
    float projection[4][4];
    float current_color[4];
    float current_normal[3];

    unsigned int texture_enabled;
    int texture0_id;
    int texture0_identity;
    float texture0_matrix[3][4];
    int texture1_id;
    int texture1_identity;
    float texture1_matrix[3][4];
    float texture1_coord[2];

    int lighting_enabled;
    int normalize_normals;
    int rescale_normals;
    int color_material_enabled;
    int color_material_mode;
    float global_ambient[4];
    float material_ambient[4];
    float material_diffuse[4];
    float material_emission[4];
    float material_specular[4];
    float material_shininess;
    WiiNativeLightSnapshot lights[WII_NATIVE_MAX_LIGHTS];
    int fog_enabled;
    int viewport[4];

    int ztest;
    int zwrite;
    unsigned char zfunc;
    int alpha_test_enabled;
    unsigned char alpha_func;
    unsigned char alpha_ref;
    int blend_enabled;
    unsigned char src_blend;
    unsigned char dst_blend;
    int cull_enabled;
    int cull_mode;
    int front_cw;
    int color_update;
    int alpha_update;
    unsigned char color_mask_r;
    unsigned char color_mask_g;
    unsigned char color_mask_b;
    unsigned char color_mask_a;
    unsigned int shade_model;
    int polygon_offset_fill;
    float polygon_offset_units;
} WiiNativeRenderStateSnapshot;

int wii_gx_get_native_state_snapshot(WiiNativeRenderStateSnapshot* out_state);
void wii_gx_get_native_modelview(float out_modelview[3][4]);
void wii_gx_invalidate_after_native_draw(void);

/* Re-programs the GX pixel engine, viewport and matrices from the tracked GL
 * state. Passes that impose their own fixed-function state on GX (a clear, a
 * terrain pass) call this when they end so their overrides do not outlive
 * them. */
void wii_gx_native_restore_pixel_engine(void);

/* Direct model-view operations for Wii renderers that no longer need to pass
 * through the OpenGL compatibility entry points. These always target the
 * model-view stack used by GX-native draws. */
int wii_gx_native_modelview_push(void);
int wii_gx_native_modelview_pop(void);
void wii_gx_native_modelview_translate(float x, float y, float z);
void wii_gx_native_modelview_rotate(float angle_degrees, float x, float y, float z);
void wii_gx_native_modelview_scale(float x, float y, float z);

/* Direct fixed-function entry points used by the Wii build. They preserve the
 * existing Beta++ render semantics while bypassing the public OpenGL-compatible
 * API at game call sites. */
void wii_gx_native_enable(unsigned int cap);
void wii_gx_native_disable(unsigned int cap);
void wii_gx_native_blend_func(unsigned int src, unsigned int dst);
void wii_gx_native_depth_mask(unsigned char enabled);
void wii_gx_native_depth_func(unsigned int func);
void wii_gx_native_alpha_func(unsigned int func, float ref);
void wii_gx_native_cull_face(unsigned int mode);
void wii_gx_native_color4f(float r, float g, float b, float a);
void wii_gx_native_color3f(float r, float g, float b);
void wii_gx_native_color_mask(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
void wii_gx_native_fogf(unsigned int pname, float value);
void wii_gx_native_fogi(unsigned int pname, int value);
void wii_gx_native_fogfv(unsigned int pname, const float* values);
void wii_gx_native_lightfv(unsigned int light, unsigned int pname, const float* values);
void wii_gx_native_light_modelfv(unsigned int pname, const float* values);
void wii_gx_native_color_material(unsigned int face, unsigned int mode);
void wii_gx_native_shade_model(unsigned int mode);
void wii_gx_native_matrix_mode(unsigned int mode);
void wii_gx_native_load_identity(void);
void wii_gx_native_matrix_push(void);
void wii_gx_native_matrix_pop(void);
void wii_gx_native_translatef(float x, float y, float z);
void wii_gx_native_rotatef(float angle, float x, float y, float z);
void wii_gx_native_scalef(float x, float y, float z);
void wii_gx_native_scaled(double x, double y, double z);
void wii_gx_native_frustum(double left, double right, double bottom, double top, double near_value, double far_value);
void wii_gx_native_ortho(double left, double right, double bottom, double top, double near_value, double far_value);
void wii_gx_native_normal3f(float x, float y, float z);
void wii_gx_native_clear(unsigned int mask);
void wii_gx_native_clear_color(float r, float g, float b, float a);
void wii_gx_native_clear_depth(double depth);
void wii_gx_native_polygon_offset(float factor, float units);
void wii_gx_native_line_width(float width);
void wii_gx_native_viewport(int x, int y, int width, int height);
void wii_gx_native_get_floatv(unsigned int pname, float* values);
const unsigned char* wii_gx_native_get_string(unsigned int name);
unsigned int wii_gx_native_get_error(void);
void wii_gx_native_hint(unsigned int target, unsigned int mode);
void wii_gx_native_set_active_texture_unit(int unit);
void wii_gx_native_set_multi_texture_coord(int unit, float u, float v);
void wii_gx_native_set_bound_texture_id(unsigned int texture);
void wii_native_state_initialize(void);

#ifdef __cplusplus
}
#endif
