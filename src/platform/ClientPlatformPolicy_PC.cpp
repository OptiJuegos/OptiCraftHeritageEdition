#include "platform/ClientPlatformPolicy.h"

#include "net/minecraft/src/GameResources.h"
#include "net/minecraft/src/GameSettings.h"
#include "pc/CrashHandler.h"
#include "platform/PlatformConfig.h"

namespace ClientPlatformPolicy
{
int initialWidth()
{
    return 854;
}

int initialHeight()
{
    return 480;
}

std::string minecraftDirectory()
{
    return GameResources::getExeDir() + "/.minecraft";
}

bool saveConverterUsesSavesSubdirectory()
{
    return true;
}

void applyGameSettingsDefaults(GameSettings* settings)
{
#if PLATFORM_PC_LEGACY
    if (settings == nullptr)
        return;

    settings->particleSetting = 2;
    settings->ofVoidParticles = false;
    settings->ofWaterParticles = false;
    settings->ofRainSplash = false;
    settings->ofPortalParticles = false;
    settings->ofDrippingWaterLava = false;
    settings->ofWeather = false;
    settings->ofSky = false;
    settings->ofSunMoon = false;
    settings->ofClouds = 3;
#else
    (void)settings;
#endif
}

void preloadStartupTextures(RenderEngine*)
{
}

void releaseWorldEntryAssets(RenderEngine*)
{
}

int panoramaSampleGrid()
{
    return 8;
}

void shutdownFlush()
{
    // The desktop GL driver serializes deletes against the queue itself.
}

void shutdownFinalize()
{
    // exit(0) hands an intact process to the OS: Windows reclaims the
    // address space only after every thread is gone, so there is nothing
    // here that must be stopped first.
}

void reportCrash(const std::string& description)
{
    CrashHandler::Crash(description);
}
}
