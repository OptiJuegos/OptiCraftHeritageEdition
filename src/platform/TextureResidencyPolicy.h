#pragma once

namespace TextureResidencyPolicy
{
// Called after RenderEngine uploads a named texture. Dynamic textures retain
// the CPU mirror required by sub-image animation updates; static textures may
// discard backend-side upload staging once the native upload is complete.
void afterNamedTextureUpload(int texture, bool dynamicTexture);
}
