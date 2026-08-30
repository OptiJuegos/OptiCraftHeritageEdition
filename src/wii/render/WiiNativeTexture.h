#pragma once

#if defined(WII_PLATFORM)
#include <gccore.h>
#include <cstdint>
#endif

bool wii_native_texture_refresh(int texture);
void wii_native_texture_generate_names(int count, int *textures);
void wii_native_texture_delete_names(int count, const int *textures);
bool wii_native_texture_bind(int texture);
void wii_native_texture_remove(int texture);
void wii_native_texture_clear_all();

#if defined(WII_PLATFORM)
bool wii_native_texture_begin_upload(int texture, int width, int height, int maxLevel,
                                     bool linear, bool clamp);
bool wii_native_texture_upload_level_rgba(int texture, int level, int width, int height,
                                          const unsigned char *pixels);
bool wii_native_texture_upload_sub_rgba(int texture, int level, int x, int y, int width, int height,
                                        const unsigned char *pixels);
bool wii_native_texture_set_parameters(int texture, bool linear, bool clamp);
bool wii_native_texture_set_quality(int texture, bool mipmapLinear, int anisotropy);
bool wii_native_texture_get_current(int *texture, GXTexObj *texobj);

#ifdef __cplusplus
extern "C" {
#endif
int wii_native_texture_get_texobj_c(int texture, GXTexObj *texobj);
#ifdef __cplusplus
}
#endif
#endif
