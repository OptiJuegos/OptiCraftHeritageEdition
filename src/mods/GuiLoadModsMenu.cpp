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

    const int_t btnW = 200;
    const int_t btnH = 20;
    const int_t centerX = width / 2 - btnW / 2;
    const int_t startY = height / 4 + 30;

    // A y-cursor keeps the layout identical on the platforms that keep the
    // USB entry (28 px rows, then the 14 px breathing gap before Back).
    int_t nextY = startY;
    controlList.push_back(new GuiButton(1, centerX, nextY, btnW, btnH, "Load from Device (Recommended)"));
    nextY += 28;
#if !defined(CTR_PLATFORM)
    // PS2/Wii mount USB mass storage; the 3DS has none, so the entry would
    // only open an empty (or failing) scan here.
    controlList.push_back(new GuiButton(2, centerX, nextY, btnW, btnH, "Load from USB Storage"));
    nextY += 28;
#endif
    controlList.push_back(new GuiButton(3, centerX, nextY + 14, btnW, btnH, "Back"));
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
