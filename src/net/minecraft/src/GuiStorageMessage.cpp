#include "GuiStorageMessage.h"
#include "GuiButton.h"
#include "FontRenderer.h"
#include "Minecraft.h"
#include "UiStrings.h"

GuiStorageMessage::GuiStorageMessage(GuiScreen *parent, GameSettings *settings, const std::string &text)
    : LegacyOptionsScreen(parent, settings), message(text) {}
void GuiStorageMessage::initGui()
{
    controlList.push_back(new GuiButton(0, width / 2 - 100, height - 42, uiText("Back")));
    syncLegacySelection();
}
void GuiStorageMessage::returnToParent() { mc->displayGuiScreen(parentScreen); }
void GuiStorageMessage::actionPerformed(GuiButton *b) { if (b && b->enabled) returnToParent(); }
void GuiStorageMessage::drawScreen(int_t x, int_t y, float_t tick)
{
    drawDefaultBackground();
    drawCenteredString(fontRenderer, uiText("Storage unavailable"), width / 2, 24, 0xffffff);
    fontRenderer->drawSplitString(uiText(message), 20, 60, width - 40, 0xffffff);
    updateLegacyPointerHover(x, y);
    GuiScreen::drawScreen(x, y, tick);
}
