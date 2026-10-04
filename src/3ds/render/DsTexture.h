#pragma once

// DsTexture.h -- texture storage for the citro3d backend.
//
// PICA200 textures must be power-of-two and between 8 and 1024 (citro3d's
// C3D_TexInitWithParams enforces both), while the game's PNGs are not always;
// each record therefore stores the rounded-up GPU size alongside the real
// image size, and the ratio is folded into the texcoord in the vertex shader
// (the texScale uniform) so UVs keep addressing the actual image.
//
// Texels are Morton-interleaved inside 8x8 tiles and the tiles are laid out
// raster-wise across each level -- the layout citro3d's own mipmap downsampler
// assumes, and the reason uploads go through here rather than through a plain
// memcpy like the framebuffer clears.

namespace ds
{
namespace texture
{

// Free every record (renderResetResources). The name allocator itself stays
// in RenderAPI_CTR_3DS.cpp, matching GL's "names survive, objects do not".
void resetAll();

// Allocate power-of-two GPU storage for a logical width x height image.
// False means the PICA cannot hold it (over 1024 either way): the caller
// reports the upload as rejected and the game falls back to vertex colour.
bool allocate(int id, int width, int height);

void destroy(int id);

// Whether this id has storage behind it (renderTextureIsValid).
bool exists(int id);

// GL_TEXTURE_MAG/MIN_FILTER and wrap mode, applied at bind time.
void setParameters(int id, bool blur, bool clamp);

// Upload a width x height RGBA byte rectangle at (x, y) of the logical image.
// A full upload is the (0, 0, w, h) case; animated textures use sub-rects.
// Level 0 only -- this backend reports no mipmap support, so nothing else is
// ever asked for.
void upload(int id, int x, int y, int width, int height, const void* rgba);

// Bind to unit 0 and write the UV padding ratio (logical/GPU size) into
// uvScale. An id without storage binds a shared 8x8 white fallback instead,
// so a never-uploaded name (GuiMainMenu's copy target, for instance) modulates
// as plain vertex colour rather than sampling undefined texels.
void bind(int id, float (&uvScale)[2]);

} // namespace texture
} // namespace ds
