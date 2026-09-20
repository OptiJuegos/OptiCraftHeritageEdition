#ifdef PS2_PLATFORM
#include "GuiWorldStorage.h"
#include "GuiStorageMessage.h"
#include "GuiButton.h"
#include "GameSettings.h"
#include "Minecraft.h"
#include "ScaledResolution.h"
#include "FontRenderer.h"
#include "UiStrings.h"
#include "ps2/storage/save/Ps2SaveStorage.h"

GuiWorldStorage::GuiWorldStorage(GuiScreen *parent, GameSettings *settings)
    : LegacyOptionsScreen(parent, settings) {}
void GuiWorldStorage::initGui()
{
    controlList.push_back(new GuiButton(1, width / 2 - 100, 108, uiText("Memory Card")));
    controlList.push_back(new GuiButton(2, width / 2 - 100, 132, uiText("USB Drive")));
    controlList.push_back(new GuiButton(0, width / 2 - 100, height - 42, uiText("Back")));
    controlList[0]->enabled = mc->theWorld == nullptr;
    controlList[1]->enabled = mc->theWorld == nullptr;
    const int selected = Ps2SaveStorage::target() == Ps2SaveStorage::Target::MassStorage ? 1 : 0;
    controlList[selected]->displayString = "> " + controlList[selected]->displayString + " <";
    syncLegacySelection();
}
void GuiWorldStorage::actionPerformed(GuiButton *b)
{
    if (!b || !b->enabled) return;
    if (b->id == 0) { returnToParent(); return; }
    if (mc->theWorld != nullptr) return; // Never redirect an active world's writes.
    if (!Ps2SaveStorage::available(Ps2SaveStorage::Target::MemoryCard))
    {
        mc->displayGuiScreen(new GuiStorageMessage(this, settings,
            "Insert a formatted Memory Card in slot 1 to save settings."));
        return;
    }
    const auto target = b->id == 2 ? Ps2SaveStorage::Target::MassStorage : Ps2SaveStorage::Target::MemoryCard;
    if (!Ps2SaveStorage::available(target))
    {
        mc->displayGuiScreen(new GuiStorageMessage(this, settings,
            "USB Drive unavailable. Connect the drive and try again."));
        return;
    }
    const auto previous = Ps2SaveStorage::target();
    Ps2SaveStorage::setTarget(target);
    settings->saveOptions();
    if (Ps2SaveStorage::configurationSaveFailed())
    {
        Ps2SaveStorage::setTarget(previous);
        mc->displayGuiScreen(new GuiStorageMessage(this, settings,
            "Could not save settings to Memory Card. Check the card and free space."));
        return;
    }
    // Refresh only the converter; existing world directories are not touched.
    mc->getSaveLoader();
    ScaledResolution sr(settings, mc->displayWidth, mc->displayHeight);
    setWorldAndResolution(mc, sr.getScaledWidth(), sr.getScaledHeight());
}
void GuiWorldStorage::drawScreen(int_t x, int_t y, float_t tick)
{
    drawDefaultBackground();
    drawCenteredString(fontRenderer, uiText("World Storage"), width / 2, 20, 0xffffff);
    fontRenderer->drawSplitString(uiText(mc->theWorld != nullptr
        ? "Leave the world before changing storage."
        : "Settings always use Memory Card. Existing worlds are not moved."),
        20, 42, width - 40, 0xffffff);
    updateLegacyPointerHover(x, y);
    GuiScreen::drawScreen(x, y, tick);
}
#endif
