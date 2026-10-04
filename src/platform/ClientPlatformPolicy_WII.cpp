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

void shutdownFlush()
{
    // Nothing to drain: the Wii teardown path returns to main_wii.cpp, whose
    // own teardown owns the GPU discipline.
}

void shutdownFinalize()
{
    // exit(0) runs libogc's exit path, which has no "unmap the heap while
    // other threads are alive" step -- nothing to stop before it.
}

void reportCrash(const std::string& description)
{
    CrashHandler::Crash(description);
}
}
