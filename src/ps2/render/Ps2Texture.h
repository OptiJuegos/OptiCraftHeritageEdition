#pragma once

#ifdef PS2_PLATFORM

// Native PS2 texture objects.
//
// This is the whole texture subsystem: name table, GS VRAM allocation, the
// RGBA -> CT16/PSMT8 conversion, the guarded gsKit upload and the deferred
// re-send of animated atlases. It used to live inside the OpenGL compatibility
// layer, reachable only through glGenTextures/glTexImage2D/glTexSubImage2D, so
// RenderEngine had to speak GL to load a PNG. Nothing here is GL-specific, and
// nothing here depends on gles_ps2.h: the compatibility entry points are now
// thin forwarders onto this API, exactly as the Wii backend does with
// wii_native_texture_*.
//
// The bound name is owned here too, so the GS draw paths resolve their texture
// from this module instead of from the emulated GL state block.
//
// This header deliberately does not include <gsKit.h>: platform/RenderAPI.h
// includes it, so it reaches every game translation unit, and gsKit's macros
// have no business there. The one entry point with a GSTEXTURE in its signature
// lives in Ps2TextureGs.h, which only the backend includes.

// Names are 1-based indices into a fixed table; 0 is "no texture".
#define PS2_MAX_TEX 256

void ps2_texture_generate_names(int count, unsigned int* names);
void ps2_texture_delete_names(int count, const unsigned int* names);

void ps2_texture_bind(unsigned int name);
unsigned int ps2_texture_bound_name();
void ps2_texture_set_tile_atlas(unsigned int name, bool tileAtlas);
bool ps2_texture_bound_is_tile_atlas();

// Wrap/filter request from the caller. The GS applies these through TEX1/TEX0
// and the per-primitive clamp selector, so they are recorded rather than
// programmed here; see ps2_select_clamp in Ps2ClipGuard.h.
void ps2_texture_set_parameters(bool linear, bool mipmaps, bool clamp);

// Specifies level 0 of `name` from tightly packed RGBA8 and sends it to GS
// VRAM. Returns false when the name is unusable or the conversion/VRAM
// reservation failed; the name is invalidated in that case so RenderEngine can
// retry the load instead of caching a permanently broken id.
bool ps2_texture_upload_rgba(unsigned int name, int level, int width, int height,
                             const void* pixels);

// Updates a sub-rectangle of `name` from tightly packed RGBA8. The GS transfer
// is deferred: the CPU-side copy is patched now and the whole texture is
// re-sent by ps2_texture_resolve() on the next draw that binds it, so the
// several animated-tile updates TextureFX issues per frame cost one upload.
bool ps2_texture_upload_sub_rgba(unsigned int name, int level, int x, int y,
                                 int width, int height, const void* pixels);

// True when `name` refers to a live, GS-uploadable texture.
bool ps2_texture_valid(unsigned int name);

// Static textures do not need a permanent EE-RAM upload mirror after a
// successful GS upload. Dynamic atlases keep it because sub-image animation
// patches that mirror before the deferred GS transfer. Returns true when the
// mirror was released.
bool ps2_texture_discard_cpu_mirror(unsigned int name);

#ifdef __cplusplus
extern "C" {
#endif


int ps2_dbg_vram_current_kb();
int ps2_dbg_vram_free_blocks();
int ps2_dbg_vram_recycled_kb();
void ps2_dbg_texture_ram_bytes(long* pixOut, long* clutOut, long* remapOut);

#ifdef __cplusplus
}
#endif

#endif // PS2_PLATFORM
