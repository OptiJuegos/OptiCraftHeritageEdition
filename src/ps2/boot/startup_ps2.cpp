#ifdef PS2_PLATFORM

#include "ps2/boot/startup_ps2.h"

#include "platform/time.h"
#include "ps2/boot/Ps2BootRenderer.h"

#include <libpad.h>
#include <tamtypes.h>

namespace
{

static bool splashCheckSkip()
{
    const int state = padGetState(0, 0);
    if (state != PAD_STATE_STABLE && state != PAD_STATE_EXECCMD)
        return false;

    padButtonStatus pad = {};
    if (!padRead(0, 0, &pad) || pad.mode == 0 || pad.btns == 0x0000)
        return false;

    static u16 previous = 0xFFFF;
    const u16 held = static_cast<u16>(0xFFFF ^ pad.btns);
    const u16 pressed = static_cast<u16>(held & ~previous);
    previous = held;
    return (pressed & PAD_START) != 0 || (pressed & PAD_CROSS) != 0;
}

static bool runSplash(const Ps2BootRenderer::Texture& texture)
{
    const int fadeInMs = 1000;
    const int showMs = 2000;
    const int fadeOutMs = 1000;
    const int totalMs = fadeInMs + showMs + fadeOutMs;

    const float width = static_cast<float>(Ps2BootRenderer::width());
    const float height = static_cast<float>(Ps2BootRenderer::height());
    const int z = 0xFFFF;

    const Ps2BootRenderer::Color black = {0, 0, 0, 0x80};
    const Ps2BootRenderer::Color neutral = {0x80, 0x80, 0x80, 0x80};

    const unsigned long long start = getTimeUS();
    for (;;)
    {
        const int elapsed = static_cast<int>((getTimeUS() - start) / 1000ULL);
        if (elapsed >= totalMs)
            break;

        unsigned char overlayAlpha = 0;
        if (elapsed < fadeInMs)
        {
            const float t = static_cast<float>(elapsed) / static_cast<float>(fadeInMs);
            overlayAlpha = static_cast<unsigned char>((1.0f - t) * 128.0f + 0.5f);
        }
        else if (elapsed >= fadeInMs + showMs)
        {
            const float t = static_cast<float>(elapsed - fadeInMs - showMs) /
                            static_cast<float>(fadeOutMs);
            overlayAlpha = static_cast<unsigned char>(t * 128.0f + 0.5f);
        }

        Ps2BootRenderer::clear(black);
        Ps2BootRenderer::setAlphaBlend(false);
        Ps2BootRenderer::drawTexture(texture,
                                     0.0f, 0.0f, 0.0f, 0.0f,
                                     width, height,
                                     static_cast<float>(texture.width),
                                     static_cast<float>(texture.height),
                                     z,
                                     neutral);

        if (overlayAlpha > 0)
        {
            Ps2BootRenderer::setAlphaBlend(true);
            Ps2BootRenderer::drawRect(0.0f,
                                      0.0f,
                                      width,
                                      height,
                                      z,
                                      {0, 0, 0, overlayAlpha});
        }

        Ps2BootRenderer::present();
        if (splashCheckSkip())
            return true;
    }

    return false;
}

} // namespace

void ps2_run_startup_logos()
{
    if (!Ps2BootRenderer::ready())
        return;

    const u32 vramBase = Ps2BootRenderer::checkpointVram();
    static const char* const names[] = {
        "startup/logo0.png",
        "startup/logo1.png"
    };

    for (const char* name : names)
    {
        Ps2BootRenderer::Texture texture;
        const bool loaded = Ps2BootRenderer::loadTextureAsset(
            name,
            texture,
            Ps2BootRenderer::TextureFilter::Linear,
            Ps2BootRenderer::TextureAlphaMode::SourceAlpha);

        if (loaded)
        {
            const bool skip = runSplash(texture);
            Ps2BootRenderer::destroyTexture(texture);
            Ps2BootRenderer::restoreVram(vramBase);
            if (skip)
                break;
        }
        else
        {
            Ps2BootRenderer::restoreVram(vramBase);
        }
    }

    Ps2BootRenderer::resetAlpha();
}

#endif // PS2_PLATFORM
