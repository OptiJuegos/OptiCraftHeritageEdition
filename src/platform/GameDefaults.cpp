#include "platform/GameDefaults.h"

#include "platform/PlatformConfig.h"
#include "platform/PlatformTuning.h"

const PlatformGameDefaults& platformGameDefaults()
{
    static const PlatformGameDefaults defaults = [] {
        PlatformGameDefaults d;
#if PLATFORM_CONSOLE_LOW || PLATFORM_WII
        d.useConsoleProfile = true;
        d.renderDistance = PLATFORM_DEFAULT_RENDER_DISTANCE;
        // Fast graphics, on the Wii for the same reason as on the PS2: fancy is
        // not a shading option here, it is a vertex-count option. BlockLeaves
        // reports isOpaqueCube() == !graphicsLevel, so fancy leaves stop culling
        // the faces inside a tree -- every interior leaf quad reaches the chunk's
        // display list, in the terrain that has the most leaves per section. It
        // also costs the second translucent pass in EntityRenderer::renderWorld.
        // Both land on the 11 MB list budget and the frame. Still a settings
        // toggle; only the default changes.
        d.fancyGraphics = false;
        d.ambientOcclusion = false;
        d.limitFramerate = 2;
        d.viewBobbing = false;
        d.fogOff = PLATFORM_WII == 0;
        d.brightness = (PLATFORM_PS2 || PLATFORM_WII) ? 1.0f : 0.0f;
        d.aoLevel = 0.0f;
        d.smoothFps = true;
        d.autoSaveTicks = 40000;
        d.weather = false;
        d.stars = false;
        d.chunkUpdates = PLATFORM_MIN_RENDERER_UPDATES_PER_FRAME;
        d.chunkUpdatesDynamic = false;
#endif
        return d;
    }();
    return defaults;
}
