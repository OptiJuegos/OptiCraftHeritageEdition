#include "net/minecraft/src/legacy/startup/StartupPresentation.h"

#include "client/Minecraft.h"
#include "java/Random.h"
#include "net/minecraft/src/GameResources.h"
#include "net/minecraft/src/GameSettings.h"
#include "net/minecraft/src/RenderEngine.h"
#include "net/minecraft/src/SoundManager.h"
#include "net/minecraft/src/Tessellator.h"
#include "net/minecraft/src/legacy/startup/StartupInput.h"
#include "net/minecraft/src/legacy/startup/StartupMusicPolicy.h"
#include "net/minecraft/src/legacy/startup/StartupPresentationPolicy.h"
#include "pc/lwjgl/Display.h"
#include "platform/Log.h"
#include "platform/PlatformConfig.h"
#include "platform/RenderAPI.h"
#include "platform/Storage.h"
#include "platform/storage/AssetPak.h"
#include "platform/time.h"

#include <string>
#include <vector>

namespace
{

constexpr int kFadeInMs = 1000;
constexpr int kHoldMs = 2000;
constexpr int kFadeOutMs = 1000;
constexpr int kTotalMs = kFadeInMs + kHoldMs + kFadeOutMs;

float overlayAlphaForElapsed(int elapsedMs)
{
    if (elapsedMs < kFadeInMs)
    {
        const float t = static_cast<float>(elapsedMs) / static_cast<float>(kFadeInMs);
        return 1.0f - t;
    }

    if (elapsedMs >= kFadeInMs + kHoldMs)
    {
        const float t = static_cast<float>(elapsedMs - kFadeInMs - kHoldMs) /
                        static_cast<float>(kFadeOutMs);
        return t < 1.0f ? t : 1.0f;
    }

    return 0.0f;
}

void prepareFrame(Minecraft* minecraft)
{
    renderViewport(0, 0, minecraft->displayWidth, minecraft->displayHeight);
    renderClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    renderClear(RenderClearMask::Color | RenderClearMask::Depth);

    renderMatrixMode(RenderMatrixMode::Projection);
    renderLoadIdentity();
    renderOrtho(0.0,
                static_cast<double>(minecraft->displayWidth),
                static_cast<double>(minecraft->displayHeight),
                0.0,
                1000.0,
                3000.0);
    renderMatrixMode(RenderMatrixMode::ModelView);
    renderLoadIdentity();
    renderTranslate(0.0f, 0.0f, -2000.0f);

    renderDisable(RenderCapability::Lighting);
    renderDisable(RenderCapability::Fog);
    renderDisable(RenderCapability::DepthTest);
    renderEnable(RenderCapability::Texture2D);
    renderEnable(RenderCapability::AlphaTest);
    renderAlphaFunc(RenderCompare::Greater, 0.1f);
    renderDisable(RenderCapability::Blend);
    renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void drawFullscreenTexture(Minecraft* minecraft, int_t texture, bool mojangSplash)
{
    renderEnable(RenderCapability::Texture2D);
    renderDisable(RenderCapability::Blend);
    renderBindTexture(texture);
    renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    Tessellator* tessellator = &Tessellator::instance;
#if PLATFORM_PS2
    if (mojangSplash)
    {
        // Preserve the square Mojang art despite the PS2's non-square pixels.
        renderClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        renderClear(RenderClearMask::Color);
        const float aspect = minecraft->gameSettings->widescreen ? 16.0f / 9.0f : 4.0f / 3.0f;
        const float h = minecraft->displayHeight * 0.75f;
        const float w = minecraft->displayWidth * 0.75f / aspect;
        const float x = (minecraft->displayWidth - w) * 0.5f;
        const float y = (minecraft->displayHeight - h) * 0.5f;
        tessellator->startDrawingQuads();
        tessellator->setColorOpaque_I(0xffffff);
        tessellator->addVertexWithUV(x, y + h, 0.0, 0.0, 1.0);
        tessellator->addVertexWithUV(x + w, y + h, 0.0, 1.0, 1.0);
        tessellator->addVertexWithUV(x + w, y, 0.0, 1.0, 0.0);
        tessellator->addVertexWithUV(x, y, 0.0, 0.0, 0.0);
        tessellator->draw();
        return;
    }
#else
    (void)mojangSplash;
#endif
    tessellator->startDrawingQuads();
    tessellator->setColorOpaque_I(0xffffff);
    tessellator->addVertexWithUV(0.0f,
                                 static_cast<float>(minecraft->displayHeight),
                                 0.0f,
                                 0.0f,
                                 1.0f);
    tessellator->addVertexWithUV(static_cast<float>(minecraft->displayWidth),
                                 static_cast<float>(minecraft->displayHeight),
                                 0.0f,
                                 1.0f,
                                 1.0f);
    tessellator->addVertexWithUV(static_cast<float>(minecraft->displayWidth),
                                 0.0f,
                                 0.0f,
                                 1.0f,
                                 0.0f);
    tessellator->addVertexWithUV(0.0f,
                                 0.0f,
                                 0.0f,
                                 0.0f,
                                 0.0f);
    tessellator->draw();
}

void drawBlackOverlay(Minecraft* minecraft, float alpha)
{
    if (alpha <= 0.0f)
        return;

    renderDisable(RenderCapability::Texture2D);
    renderDisable(RenderCapability::AlphaTest);
    renderEnable(RenderCapability::Blend);
    renderBlendFunc(RenderBlendFactor::SrcAlpha, RenderBlendFactor::OneMinusSrcAlpha);
    renderColor4f(0.0f, 0.0f, 0.0f, alpha);

    Tessellator* tessellator = &Tessellator::instance;
    tessellator->startDrawingQuads();
    tessellator->addVertex(0.0f, static_cast<float>(minecraft->displayHeight), 0.0f);
    tessellator->addVertex(static_cast<float>(minecraft->displayWidth),
                           static_cast<float>(minecraft->displayHeight),
                           0.0f);
    tessellator->addVertex(static_cast<float>(minecraft->displayWidth), 0.0f, 0.0f);
    tessellator->addVertex(0.0f, 0.0f, 0.0f);
    tessellator->draw();
    renderEnable(RenderCapability::AlphaTest);
}

void restoreRenderState()
{
    renderEnable(RenderCapability::Texture2D);
    renderDisable(RenderCapability::Blend);
    renderEnable(RenderCapability::AlphaTest);
    renderAlphaFunc(RenderCompare::Greater, 0.1f);
    renderEnable(RenderCapability::DepthTest);
    renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    renderMatrixMode(RenderMatrixMode::Projection);
    renderLoadIdentity();
    renderMatrixMode(RenderMatrixMode::ModelView);
    renderLoadIdentity();
}

// Fills `out` with the playable paths of every calm track: pak:// paths from
// the mounted pak, loose paths otherwise. False when no music folder exists.
bool collectCalmTracks(std::vector<std::string> &out)
{
    const char* const kPakMusicKey = "resources/music";
    std::vector<std::string> entries;
    if (AssetPak::mounted() && AssetPak::listChildren(kPakMusicKey, entries))
    {
        for (const std::string &name : entries)
        {
            const std::string key = std::string(kPakMusicKey) + "/" + name;
            if (AssetPak::exists(key) && LegacyStartup::isCalmTrackFilename(name))
                out.push_back(AssetPak::makePath(key));
        }
        return true;
    }

    const std::string musicDirectory = PlatformStorage::join(
        GameResources::getAudioResourcesDir(), "music");
    if (!PlatformStorage::listPathEntries(musicDirectory, entries))
    {
        MC_LOG_WARN("client.startup", "Legacy startup music directory missing: %s\n",
                    musicDirectory.c_str());
        return false;
    }

    out.reserve(entries.size());
    for (const std::string &name : entries)
    {
        const std::string path = PlatformStorage::join(musicDirectory, name);
        if (!PlatformStorage::pathIsDirectory(path) && LegacyStartup::isCalmTrackFilename(name))
            out.push_back(path);
    }
    return true;
}

bool startLegacyCalmMusic(Minecraft* minecraft)
{
    if (minecraft->sndManager == nullptr ||
        !LegacyStartup::shouldStartCalmMusic(minecraft->gameSettings->legacyUI,
                                             minecraft->gameSettings->musicVolume))
        return false;

    // This runs before ThreadDownloadResources registers the sound pool, so
    // the tracks are found by listing the music folder directly.
    std::vector<std::string> calmTracks;
    if (!collectCalmTracks(calmTracks))
        return false;

    if (calmTracks.empty())
    {
        MC_LOG_WARN("client.startup", "No calm music found under resources/music\n");
        return false;
    }

    Random random;
    while (!calmTracks.empty())
    {
        const int_t index = random.nextInt(static_cast<int_t>(calmTracks.size()));
        const std::string path = calmTracks[static_cast<std::size_t>(index)];
        calmTracks[static_cast<std::size_t>(index)] = calmTracks.back();
        calmTracks.pop_back();

        if (minecraft->sndManager->playMusicFileNow(path))
        {
            MC_LOG_DEBUG("client.startup", "Legacy startup music: %s\n", path.c_str());
            return true;
        }
    }

    MC_LOG_WARN("client.startup", "No playable calm music found for this platform\n");
    return false;
}

enum class LogoResult
{
    Missing,
    Completed,
    Skipped
};

LogoResult playLegacyLogo(Minecraft* minecraft, const char* resourcePath, bool &musicStarted)
{
    if (!minecraft->renderEngine->hasResource(resourcePath))
    {
        MC_LOG_WARN("client.startup", "Legacy startup logo missing: %s\n", resourcePath);
        return LogoResult::Missing;
    }

    const int_t texture = minecraft->renderEngine->getTexture(resourcePath);
    const unsigned long long start = getTimeUS();
    bool firstFramePresented = false;

    for (;;)
    {
        const int elapsedMs = static_cast<int>((getTimeUS() - start) / 1000ULL);
        if (elapsedMs >= kTotalMs)
            break;

        prepareFrame(minecraft);
        drawFullscreenTexture(minecraft, texture, std::string(resourcePath) == "/title/mojang.png");
        drawBlackOverlay(minecraft, overlayAlphaForElapsed(elapsedMs));
        lwjgl::Display::swapBuffers();

        if (!firstFramePresented)
        {
            firstFramePresented = true;
            if (!musicStarted)
                musicStarted = startLegacyCalmMusic(minecraft);
        }

        if (LegacyStartup::pollStartupSkipRequested())
        {
            restoreRenderState();
            minecraft->renderEngine->releaseTexture(resourcePath);
            return LogoResult::Skipped;
        }
    }

    restoreRenderState();
    minecraft->renderEngine->releaseTexture(resourcePath);
    return LogoResult::Completed;
}

} // namespace

namespace LegacyStartup
{

void run(Minecraft* minecraft)
{
    if (minecraft == nullptr || minecraft->gameSettings == nullptr || minecraft->renderEngine == nullptr)
        return;

    if (selectPresentation(minecraft->gameSettings->legacyUI) == PresentationMode::Java)
    {
        minecraft->loadScreen();
        return;
    }

    if (minecraft->sndManager != nullptr)
        minecraft->sndManager->loadSoundSettings(minecraft->gameSettings);

    beginStartupInput();
    bool musicStarted = false;

    LogoResult first = playLegacyLogo(minecraft, "/legacy/logo1.png", musicStarted);
    if (first == LogoResult::Skipped)
        return;

    const LogoResult second = playLegacyLogo(minecraft,
#if PLATFORM_PS2
        "/title/mojang.png",
#else
        "/legacy/logo2.png",
#endif
        musicStarted);
    (void)second;
}

} // namespace LegacyStartup
