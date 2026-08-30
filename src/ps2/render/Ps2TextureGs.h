#pragma once

#ifdef PS2_PLATFORM

#include <gsKit.h>

#include "ps2/render/Ps2Texture.h"

// The gsKit-typed half of the texture module, for the GS draw paths only.
// Ps2Texture.h stays free of gsKit because platform/RenderAPI.h includes it and
// therefore so does every game translation unit.

// Returns the GSTEXTURE to draw with, flushing any deferred sub-image upload
// first. Returns nullptr when the name has no usable texture.
GSTEXTURE* ps2_texture_resolve(unsigned int name);

#endif // PS2_PLATFORM
