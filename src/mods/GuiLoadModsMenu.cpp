#include "GuiLoadModsMenu.h"
#include "GuiButton.h"
#include "Minecraft.h"
#include "FontRenderer.h"
#include "GuiLoadModsList.h"

#if PLATFORM_PS2 || PLATFORM_WII
#include "platform/Input.h"
#endif

GuiLoadModsMenu::GuiLoadModsMenu(GuiScreen *parent)
    : parentScreen(parent)
{
}

void GuiLoadModsMenu::initGui()
{
    controlList.clear();

    int_t btnW = 200;
    int_t btnH = 20;
    int_t centerX = width / 2 - btnW / 2;
    int_t startY = height / 4 + 30;

    controlList.push_back(new GuiButton(1, centerX, startY, btnW, btnH, "Load from Device"));
    controlList.push_back(new GuiButton(2, centerX, startY + 28, btnW, btnH, "Load from USB"));
    controlList.push_back(new GuiButton(3, centerX, startY + 70, btnW, btnH, "Back"));
}

void GuiLoadModsMenu::actionPerformed(GuiButton *button)
{
    if (!button->enabled)
        return;

    if (button->id == 1) // Device
    {
        mc->displayGuiScreen(new GuiLoadModsList(this, GuiLoadModsList::Source::Device));
    }
    else if (button->id == 2) // USB
    {
        mc->displayGuiScreen(new GuiLoadModsList(this, GuiLoadModsList::Source::USB));
    }
    else if (button->id == 3) // Back
    {
        mc->displayGuiScreen(parentScreen);
    }
}

void GuiLoadModsMenu::keyTyped(char_t c, int_t key)
{
    if (key == 1) // ESC
    {
        mc->displayGuiScreen(parentScreen);
        return;
    }
    GuiScreen::keyTyped(c, key);
}

void GuiLoadModsMenu::handleSpecializedMenuInput()
{
#if PLATFORM_PS2 || PLATFORM_WII
    const PlatformTextInputSnapshot pad = platformTextInputSnapshot(platformMenuPad());
    if ((pad.pressed & (PLATFORM_TEXT_BACK | PLATFORM_TEXT_CLOSE)) != 0)
    {
        mc->displayGuiScreen(parentScreen);
        return;
    }
#endif
}

void GuiLoadModsMenu::drawScreen(int_t mouseX, int_t mouseY, float_t partialTick)
{
    drawDefaultBackground();

    drawCenteredString(fontRenderer, "Load Mods (.ochpack)", width / 2, 25, 0xFFFFFF);
    drawCenteredString(fontRenderer, std::string("\xc2\xa7") + "7Select the storage location to scan for mods", width / 2, 40, 0x888888);

    GuiScreen::drawScreen(mouseX, mouseY, partialTick);
}
