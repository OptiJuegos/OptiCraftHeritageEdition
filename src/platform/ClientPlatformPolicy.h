#pragma once

#include <string>

class GameSettings;
class RenderEngine;

namespace ClientPlatformPolicy
{
    int initialWidth();
    int initialHeight();
    std::string minecraftDirectory();
    bool saveConverterUsesSavesSubdirectory();
    void applyGameSettingsDefaults(GameSettings* settings);
    void preloadStartupTextures(RenderEngine* renderEngine);
    void releaseWorldEntryAssets(RenderEngine* renderEngine);
    int panoramaSampleGrid();
    bool showInitialLoadScreen();
    void reportCrash(const std::string& description);
}
