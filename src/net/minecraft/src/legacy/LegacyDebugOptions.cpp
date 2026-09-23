#include "net/minecraft/src/UiStrings.h"
#include "LegacyDebugOptions.h"

#include "LegacyGuiButton.h"
#include "LegacyOptionCheckbox.h"
#include "LegacyOptionLabel.h"
#include "net/minecraft/src/Entity.h"
#include "net/minecraft/src/EntityDragon.h"
#include "net/minecraft/src/EntityPlayerSP.h"
#include "net/minecraft/src/GameSettings.h"
#include "net/minecraft/src/GuiButton.h"
#include "net/minecraft/src/Minecraft.h"
#include "net/minecraft/src/PlayerController.h"
#include "net/minecraft/src/PlayerControllerCreative.h"
#include "net/minecraft/src/PlayerControllerSP.h"
#include "net/minecraft/src/World.h"
#include "net/minecraft/src/WorldInfo.h"
#include "pc/lwjgl/Keyboard.h"

namespace
{
enum LegacyDebugButtonId
{
    BUTTON_SHOW_FPS = 700,
    BUTTON_EXTENDED_INFO = 701,
    BUTTON_SET_DAY = 702,
    BUTTON_GAME_MODE = 703,
    BUTTON_KEEP_INVENTORY = 704,
    BUTTON_KILL_ENTITIES = 705,
    BUTTON_DONE = 799
};

constexpr long_t TICKS_PER_DAY = 24000LL;
constexpr long_t DAY_TIME = 1000LL;
}

LegacyDebugOptions::LegacyDebugOptions(GuiScreen *parent, GameSettings *settingsValue,
    LegacyOptionsBackgroundMode backgroundModeValue)
    : LegacyOptionsScreen(parent, settingsValue, backgroundModeValue),
      showFpsCheckbox(nullptr), extendedInfoCheckbox(nullptr), keepInventoryCheckbox(nullptr),
      setDayButton(nullptr), gameModeButton(nullptr), killEntitiesButton(nullptr), multiplayer(false)
{
}

void LegacyDebugOptions::initGui()
{
    configureLegacyLayout(8, true, LegacyOptionsLayoutPreset::Compact);
    const int_t x = legacyLayout.contentX;
    const int_t w = legacyLayout.contentWidth;
    const int_t h = legacyLayout.rowHeight;

    multiplayer = mc != nullptr && mc->isMultiplayerWorld();

    showFpsCheckbox = new LegacyOptionCheckbox(BUTTON_SHOW_FPS, x, legacyLayout.rowY(0), w, h,
        uiText("Show FPS"), settings->showFps);
    extendedInfoCheckbox = new LegacyOptionCheckbox(BUTTON_EXTENDED_INFO, x, legacyLayout.rowY(1), w, h,
        uiText("F3 Extended Info"), settings->showDebugInfo);
    setDayButton = new LegacyGuiButton(BUTTON_SET_DAY, x, legacyLayout.rowY(3), w, h, uiText("Set Day"));
    gameModeButton = new LegacyGuiButton(BUTTON_GAME_MODE, x, legacyLayout.rowY(4), w, h, "");
    keepInventoryCheckbox = new LegacyOptionCheckbox(BUTTON_KEEP_INVENTORY, x, legacyLayout.rowY(5), w, h,
        uiText("Keep Inventory"), settings->debugKeepInventory);
    killEntitiesButton = new LegacyGuiButton(BUTTON_KILL_ENTITIES, x, legacyLayout.rowY(6), w, h, uiText("Kill Entities"));

    setDayButton->enabled = !multiplayer;
    gameModeButton->enabled = !multiplayer;
    keepInventoryCheckbox->enabled = !multiplayer;
    killEntitiesButton->enabled = !multiplayer;

    controlList.push_back(showFpsCheckbox);
    controlList.push_back(extendedInfoCheckbox);
    controlList.push_back(setDayButton);
    controlList.push_back(gameModeButton);
    controlList.push_back(keepInventoryCheckbox);
    controlList.push_back(killEntitiesButton);
    controlList.push_back(new LegacyGuiButton(BUTTON_DONE, x, legacyLayout.rowY(7), w, h, uiText("Done")));

    syncControls();
}

void LegacyDebugOptions::syncControls()
{
    if (showFpsCheckbox != nullptr)
        showFpsCheckbox->setChecked(settings->showFps);
    if (extendedInfoCheckbox != nullptr)
        extendedInfoCheckbox->setChecked(settings->showDebugInfo);
    if (keepInventoryCheckbox != nullptr)
        keepInventoryCheckbox->setChecked(settings->debugKeepInventory);
    if (gameModeButton != nullptr)
    {
        const bool creative = mc != nullptr && mc->playerController != nullptr &&
            mc->playerController->isInCreativeMode();
        gameModeButton->displayString = std::string(uiText("Game Mode: ")) + (creative ? uiText("Creative") : uiText("Survival"));
    }
}

void LegacyDebugOptions::setDay()
{
    if (multiplayer || mc == nullptr || mc->theWorld == nullptr)
        return;

    const long_t time = mc->theWorld->getWorldTime();
    const long_t dayStart = time - time % TICKS_PER_DAY;
    mc->theWorld->setWorldTime(dayStart + DAY_TIME);
}

void LegacyDebugOptions::killEntities()
{
    if (multiplayer || mc == nullptr || mc->theWorld == nullptr)
        return;

    std::vector<Entity *> entities = mc->theWorld->getLoadedEntityList();
    for (Entity *entity : entities)
    {
        if (entity == nullptr || entity->isPlayer() || entity->isDead)
            continue;

        EntityDragon *dragon = dynamic_cast<EntityDragon *>(entity);
        if (dragon != nullptr)
        {
            dragon->setHealth(0);
            continue;
        }

        mc->theWorld->setEntityDead(entity);
    }
}

void LegacyDebugOptions::setCreativeMode(bool creative)
{
    if (multiplayer || mc == nullptr || mc->theWorld == nullptr || mc->thePlayer == nullptr)
        return;

    const bool alreadyCreative = mc->playerController != nullptr && mc->playerController->isInCreativeMode();
    if (alreadyCreative == creative)
        return;

    PlayerController *oldController = mc->playerController;
    mc->playerController = creative
        ? static_cast<PlayerController *>(new PlayerControllerCreative(mc))
        : static_cast<PlayerController *>(new PlayerControllerSP(mc));
    delete oldController;

    mc->playerController->onWorldChanged(mc->theWorld);
    mc->playerController->initializePlayer(mc->thePlayer);

    WorldInfo *worldInfo = mc->theWorld->getWorldInfo();
    if (worldInfo != nullptr)
        worldInfo->setGameType(creative ? 1 : 0);
}

void LegacyDebugOptions::actionPerformed(GuiButton *button)
{
    if (button == nullptr || !button->enabled)
        return;

    switch (button->id)
    {
    case BUTTON_SHOW_FPS:
        settings->showFps = !settings->showFps;
        syncControls();
        return;
    case BUTTON_EXTENDED_INFO:
        settings->showDebugInfo = !settings->showDebugInfo;
        syncControls();
        return;
    case BUTTON_SET_DAY:
        setDay();
        return;
    case BUTTON_GAME_MODE:
        setCreativeMode(mc != nullptr && mc->playerController != nullptr &&
            !mc->playerController->isInCreativeMode());
        syncControls();
        return;
    case BUTTON_KEEP_INVENTORY:
        settings->debugKeepInventory = !settings->debugKeepInventory;
        syncControls();
        return;
    case BUTTON_KILL_ENTITIES:
        killEntities();
        return;
    case BUTTON_DONE:
        returnToParent();
        return;
    default:
        return;
    }
}

void LegacyDebugOptions::keyTyped(char_t c, int_t key)
{
    if (key == lwjgl::Keyboard::KEY_F3)
    {
        mc->displayGuiScreen(nullptr);
        return;
    }
    LegacyOptionsScreen::keyTyped(c, key);
}

void LegacyDebugOptions::drawScreen(int_t mouseX, int_t mouseY, float_t partialTick)
{
    drawLegacyBackground(partialTick);
    legacyDrawOptionLabel(fontRenderer, uiText("Debug Cheats"), legacyLayout.contentX,
        legacyLayout.rowY(2) + (legacyLayout.rowHeight - 8) / 2);
    updateLegacyPointerHover(mouseX, mouseY);
    GuiScreen::drawScreen(mouseX, mouseY, partialTick);
}
