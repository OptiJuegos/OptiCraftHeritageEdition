#include "net/minecraft/src/UiStrings.h"
#include "GuiCredits.h"

#include "CreditsContent.h"
#include "GameSettings.h"
#include "GuiButton.h"
#include "Minecraft.h"
#include "legacy/LegacyGuiButton.h"
#include "legacy/LegacyMenuHints.h"

GuiCredits::GuiCredits(GuiScreen *parent, GameSettings *settings)
    : LegacyOptionsScreen(parent, settings)
{
}

void GuiCredits::initGui()
{
    const int_t y = CreditsContent::backY(height);
    if (settings != nullptr && settings->legacyUI)
        controlList.push_back(new LegacyGuiButton(0, width / 2 - 100, y, 200,
            CreditsContent::backHeight, uiText("Back")));
    else
        controlList.push_back(new GuiButton(0, width / 2 - 100, y, 200,
            CreditsContent::backHeight, uiText("Back")));
    syncLegacySelection();
}

void GuiCredits::actionPerformed(GuiButton *button)
{
    if (button != nullptr && button->enabled && button->id == 0)
        returnToParent();
}

void GuiCredits::returnToParent()
{
    // Credits changes no settings and should not write to the save device.
    mc->displayGuiScreen(parentScreen);
}

void GuiCredits::drawScreen(int_t mouseX, int_t mouseY, float_t partialTick)
{
    drawDefaultBackground();
    drawCenteredString(fontRenderer, uiText("Credits"), width / 2, 20, 0xffffff);
    int_t y = CreditsContent::textTop;
    for (const char *line : CreditsContent::lines)
    {
        drawCenteredString(fontRenderer, uiText(line), width / 2, y, 0xffffff);
        y += CreditsContent::lineHeight;
    }
    updateLegacyPointerHover(mouseX, mouseY);
    GuiScreen::drawScreen(mouseX, mouseY, partialTick);
    drawLegacyMenuHints(fontRenderer, width, height, true);
}
