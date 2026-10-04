#include "platform/GameDefaults.h"

#include "platform/PlatformConfig.h"
#include "platform/PlatformTuning.h"

const PlatformGameDefaults& platformGameDefaults()
{
    static const PlatformGameDefaults defaults = [] {
        PlatformGameDefaults d;
// The 3DS joins the profile list: without the claim here the GameSettings
// constructor keeps its desktop defaults (renderDistance 0 = FAR,
// ofRenderDistanceFine 128, fancy graphics, smooth lighting), so the
// carefully-tuned DsWorldTuning table never runs and RenderGlobal builds the
// 17 x 8 x 17 = 2312-section grid the tuning header itself calls unaffordable
// on this hardware -- the world-entry frames of multiple seconds, the sky
// pass rendering at "renderDistance < 2", and the mesh storms the camera-
// movement freeze reports all trace to that unclaimed branch.
#if PLATFORM_CONSOLE_LOW || PLATFORM_WII || PLATFORM_PC_LEGACY || PLATFORM_3DS
        d.usePerformanceProfile = true;
        d.renderDistance = PLATFORM_DEFAULT_RENDER_DISTANCE;
        // Fast graphics reduce terrain vertex count: BlockLeaves reports
        // isOpaqueCube() == !graphicsLevel, so interior leaf faces are culled
        // instead of entering the chunk mesh and the translucent terrain pass.
        // This remains a normal setting toggle; only the profile default changes.
        d.fancyGraphics = false;
        // Smooth lighting off on both consoles: each AO face samples nine
        // light values and blends four corner brightnesses, which is most of
        // the per-face cost of RenderBlocks on a chunk build. The setting stays
        // available in the options.
        d.ambientOcclusion = false;
#if PLATFORM_PC_LEGACY
        d.particleSetting = 2;
#elif PLATFORM_WII
        // Decreased: spawnParticle drops a third of the requests. A block
        // break alone is 4x4x4 EntityFX, each a live entity with its own
        // collision sweep every tick; PS2 skips particles outright.
        d.particleSetting = 1;
#elif PLATFORM_3DS
        // Same reasoning as the Wii: every particle is a live entity with a
        // per-tick collision sweep, and its draw is a fresh tessellation the
        // ARM11 pays for in full. Decreased keeps the visible ones.
        d.particleSetting = 1;
#else
        d.particleSetting = 0;
#endif
#if PLATFORM_WII
        // Balanced: the GX swap already waits for vsync (gx_wii.cpp), so the
        // Power saver sleep before the swap only pushes frames to the next
        // vblank. Chunk updates stay bounded by the per-frame limit either way.
        d.limitFramerate = 1;
#elif PLATFORM_3DS
        // Same reasoning as the Wii, plus one trap the Wii does not have:
        // the main-menu branch of the framerate limiter runs ONLY for
        // limitFramerate == 2 (EntityRenderer.cpp), and it computes its sleep
        // as frame-timestamp-minus-steady_clock -- two clocks with different
        // bases, which under Azahar sit far enough apart that the result
        // lands just under the branch's own 500 ms clamp every frame. That
        // is the 0-1 fps main menu this profile's first build shipped with.
        // == 1 leaves the menu unthrottled (that branch does not run) and
        // lets the citro3d SYNCDRAW pacing own the frame rate, exactly as
        // it did before this profile was claimed.
        d.limitFramerate = 1;
#else
        d.limitFramerate = 2;
#endif
#if PLATFORM_PS2 || PLATFORM_WII
        d.viewBobbing = true;
#else
        d.viewBobbing = false;
#endif
        d.fogOff = PLATFORM_PS2 != 0;
        d.brightness = (PLATFORM_PS2 || PLATFORM_WII) ? 1.0f : 0.0f;
        d.aoLevel = 0.0f;
#if PLATFORM_PC_LEGACY
        d.smoothFps = false;
#else
        d.smoothFps = true;
#endif
        d.autoSaveTicks = 40000;
        d.weather = false;
#if PLATFORM_PC_LEGACY
        d.sky = false;
        d.sunMoon = false;
        d.clouds = 3;
#elif PLATFORM_PS2 || PLATFORM_WII || PLATFORM_3DS
        d.clouds = 3;
#else
        d.clouds = 1;
#endif
        d.stars = false;
        d.chunkUpdates = PLATFORM_MIN_RENDERER_UPDATES_PER_FRAME;
        d.chunkUpdatesDynamic = false;
        d.mipmapLevel = PLATFORM_DEFAULT_MIPMAP_LEVEL;
#endif
        return d;
    }();
    return defaults;
}
