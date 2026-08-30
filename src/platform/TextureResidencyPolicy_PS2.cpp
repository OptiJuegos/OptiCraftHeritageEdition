#include "platform/TextureResidencyPolicy.h"

#include "ps2/render/Ps2Texture.h"

namespace TextureResidencyPolicy
{
void afterNamedTextureUpload(int texture, bool dynamicTexture)
{
    if (!dynamicTexture && texture > 0)
        ps2_texture_discard_cpu_mirror(static_cast<unsigned int>(texture));
}
}
