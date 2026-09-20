#include "net/minecraft/src/UiStrings.h"
#include "LegacyTutorialWorld.h"

#include "LegacyTipHud.h"

#include <memory>
#include <string>
#include <vector>

#include "net/minecraft/src/AnvilSaveConverter.h"
#include "net/minecraft/src/ISaveFormat.h"
#include "net/minecraft/src/Minecraft.h"
#include "net/minecraft/src/PlayerControllerCreative.h"
#include "net/minecraft/src/PlayerControllerSP.h"
#include "net/minecraft/src/WorldInfo.h"
#include "platform/Log.h"
#include "platform/Resources.h"
#include "platform/Storage.h"
#include "platform/storage/AssetPak.h"

namespace
{
const char *const TUTORIAL_ASSET_DIRECTORY = "legacy";
const char *const TUTORIAL_WORLD_DIRECTORY = "tutorial";

// The world is opened in place, read-only, from wherever it ships: inside
// assets.pak when the pak carries it (PlatformStorage and RegionFile read
// "pak://" paths), else the loose assets/legacy tree.
std::string tutorialRoot()
{
    const std::string pakRoot = std::string("assets/") + TUTORIAL_ASSET_DIRECTORY;
    if (AssetPak::mounted() &&
        AssetPak::exists(pakRoot + "/" + TUTORIAL_WORLD_DIRECTORY + "/level.dat"))
        return AssetPak::makePath(pakRoot);
    return PlatformStorage::join(PlatformResources::assetsDir(), TUTORIAL_ASSET_DIRECTORY);
}

std::string tutorialWorldPath()
{
    return PlatformStorage::join(tutorialRoot(), TUTORIAL_WORLD_DIRECTORY);
}

bool validateTutorialWorld(ISaveFormat *saveFormat, std::string &errorMessage)
{
    errorMessage.clear();
    if (saveFormat == nullptr)
    {
        errorMessage = uiText("Tutorial world loader is not available");
        return false;
    }

    const std::string worldPath = tutorialWorldPath();
    if (!PlatformStorage::pathIsDirectory(worldPath))
    {
        errorMessage = "Missing assets/legacy/tutorial";
        return false;
    }

    const std::string levelPath = PlatformStorage::join(worldPath, "level.dat");
    if (!PlatformStorage::exists(levelPath))
    {
        errorMessage = "Missing assets/legacy/tutorial/level.dat";
        return false;
    }

    const std::string regionPath = PlatformStorage::join(worldPath, "region");
    if (!PlatformStorage::pathIsDirectory(regionPath))
    {
        errorMessage = "Missing assets/legacy/tutorial/region";
        return false;
    }

    std::unique_ptr<WorldInfo> info(saveFormat->getWorldInfo(TUTORIAL_WORLD_DIRECTORY));
    if (info == nullptr)
    {
        errorMessage = uiText("Tutorial level.dat could not be parsed");
        return false;
    }

    const int_t saveVersion = info->getSaveVersion();
    MC_LOG_INFO("tutorial", "direct world='%s' saveVersion=%d worldName='%s'\n",
        worldPath.c_str(), static_cast<int>(saveVersion), info->getWorldName().c_str());
    if (saveVersion != 19133)
    {
        errorMessage = uiText("Tutorial world must be pre-converted to Anvil 19133");
        return false;
    }

    return true;
}
}

namespace LegacyTutorialWorld
{
const std::string &saveDirectoryName()
{
    static const std::string value = TUTORIAL_WORLD_DIRECTORY;
    return value;
}

const std::string &displayName()
{
    static std::string value;
    value = uiText("Tutorial World");
    return value;
}

bool ensureInstalled(Minecraft *mc, std::string &errorMessage)
{
    if (mc == nullptr)
    {
        errorMessage = uiText("Tutorial world loader is not available");
        return false;
    }

    AnvilSaveConverter directFormat(tutorialRoot(), true);
    return validateTutorialWorld(&directFormat, errorMessage);
}

bool play(Minecraft *mc, std::string &errorMessage)
{
    errorMessage.clear();
    if (mc == nullptr)
    {
        errorMessage = uiText("Tutorial world loader is not available");
        return false;
    }

    std::unique_ptr<AnvilSaveConverter> directFormat(new AnvilSaveConverter(tutorialRoot(), true));
    if (!validateTutorialWorld(directFormat.get(), errorMessage))
        return false;

    std::unique_ptr<WorldInfo> info(directFormat->getWorldInfo(saveDirectoryName()));
    if (info == nullptr)
    {
        errorMessage = uiText("Tutorial world could not be opened");
        return false;
    }

    delete mc->playerController;
    mc->playerController = info->getGameType() == 0
        ? static_cast<PlayerController *>(new PlayerControllerSP(mc))
        : static_cast<PlayerController *>(new PlayerControllerCreative(mc));

    Minecraft::WorldLoadOptions loadOptions;
    loadOptions.naturalMobSpawningEnabled = false;

    mc->displayGuiScreen(nullptr);
    mc->startWorld(directFormat.get(), saveDirectoryName(), displayName(), static_cast<WorldSettings *>(nullptr),
                   loadOptions);
    mc->displayGuiScreen(nullptr);
    if (mc->theWorld == nullptr)
        return false;

    // Legacy Console greets the player on entering the tutorial with a tip
    // window; the HUD starts its timer on the first frame it is drawn.
    LegacyTipHud::clear();
    LegacyTipHud::show(uiText("Welcome to OptiCraft Heritage! Enjoy exploring this world :)!"));
    return true;
}
}
