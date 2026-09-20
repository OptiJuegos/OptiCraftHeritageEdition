#include "platform/ClientPlatformPolicy.h"

#include "net/minecraft/src/GameSettings.h"
#include "net/minecraft/src/RenderEngine.h"
#include "platform/Log.h"
#include "ps2/storage/save/Ps2SaveStorage.h"

namespace ClientPlatformPolicy
{
int initialWidth()
{
    return 640;
}

int initialHeight()
{
    return 448;
}

std::string minecraftDirectory()
{
    return Ps2SaveStorage::configRoot();
}

bool saveConverterUsesSavesSubdirectory()
{
    return false;
}

void applyGameSettingsDefaults(GameSettings* settings)
{
    if (settings == nullptr)
        return;

    settings->ofAnimatedWater = 2;
    settings->ofAnimatedLava = 2;
    settings->ofAnimatedFire = false;
    settings->ofAnimatedPortal = false;
    settings->ofAnimatedRedstone = false;
    settings->ofAnimatedExplosion = false;
    settings->ofAnimatedFlame = false;
    settings->ofAnimatedSmoke = false;
    settings->advancedOpengl = false;
    settings->ofOcclusionFancy = false;
}

void preloadStartupTextures(RenderEngine* renderEngine)
{
    // PS2 loads UI textures on first use. Preloading them here permanently
    // raised the startup high-water mark and kept menu-only resources resident
    // before the menu actually needed them.
    (void)renderEngine;
}

void releaseWorldEntryAssets(RenderEngine* renderEngine)
{
    if (renderEngine == nullptr)
        return;

    // GuiMainMenu remains alive as the parent of GuiSelectWorld/GuiCreateWorld
    // until after startWorld() returns, so waiting for its destructor keeps the
    // cubemap resident during the heaviest allocation window. Release menu-only
    // textures before the World constructor instead; getTexture() reloads them
    // lazily when the main menu is shown again.
    for (int face = 0; face < 6; ++face)
        renderEngine->releaseTexture(
            "/title/bg/panorama" + std::to_string(face) + ".png");
    renderEngine->releaseTexture("/legacy/panorama.png");
    renderEngine->releaseTexture("/title/mclogo.png");

    // readTextureImageData() keeps at most one decoded helper image on PS2 for
    // short back-to-back reuse. No decoded helper is needed while constructing
    // a world, so return that last buffer to the heap as well.
    renderEngine->clearDecodedTextureCache();
}

int panoramaSampleGrid()
{
    return 2;
}


void reportCrash(const std::string& description)
{
    MC_LOG_ERROR("crash", "%s\n", description.c_str());
}
}
