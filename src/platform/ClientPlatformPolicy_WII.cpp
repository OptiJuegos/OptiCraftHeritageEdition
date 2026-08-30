#include "platform/ClientPlatformPolicy.h"

#include "net/minecraft/src/GameResources.h"
#include "pc/CrashHandler.h"
#include "wii/gx_wii.h"

namespace ClientPlatformPolicy
{
int initialWidth()
{
    return wiigl_width();
}

int initialHeight()
{
    return wiigl_height();
}

std::string minecraftDirectory()
{
    return GameResources::getExeDir() + "/.minecraft";
}

bool saveConverterUsesSavesSubdirectory()
{
    return true;
}

void applyGameSettingsDefaults(GameSettings*)
{
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

bool showInitialLoadScreen()
{
    return true;
}

void reportCrash(const std::string& description)
{
    CrashHandler::Crash(description);
}
}
