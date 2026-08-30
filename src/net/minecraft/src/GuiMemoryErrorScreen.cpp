#include "GuiMemoryErrorScreen.h"

#include "GuiButton.h"
#include "GuiMainMenu.h"
#include "GuiSmallButton.h"
#include "Minecraft.h"
#include "StringTranslate.h"

void GuiMemoryErrorScreen::updateScreen()
{
}

void GuiMemoryErrorScreen::initGui()
{
    StringTranslate *translate = StringTranslate::getInstance();
    controlList.push_back(new GuiSmallButton(0, width / 2 - 155, height / 4 + 132,
                                             translate->translateKey("gui.toMenu")));
    controlList.push_back(new GuiSmallButton(1, width / 2 + 5, height / 4 + 132,
                                             translate->translateKey("menu.quit")));
}

void GuiMemoryErrorScreen::actionPerformed(GuiButton *button)
{
    if (button->id == 0)
        mc->displayGuiScreen(new GuiMainMenu());
    else if (button->id == 1)
        mc->shutdown();
}

void GuiMemoryErrorScreen::keyTyped(char_t, int_t)
{
}

void GuiMemoryErrorScreen::drawScreen(int_t mouseX, int_t mouseY, float_t partialTick)
{
    drawDefaultBackground();
    int_t baseX = width / 2 - 140;
    int_t baseY = height / 4;
    drawCenteredString(fontRenderer, "Out of memory!", width / 2, baseY - 40, 0xffffff);
    drawString(fontRenderer, "Minecraft has run out of memory.", baseX, baseY, 0xa0a0a0);
    drawString(fontRenderer, "This could be caused by a bug in the game or by the", baseX, baseY + 18, 0xa0a0a0);
    drawString(fontRenderer, "Java Virtual Machine not being allocated enough", baseX, baseY + 27, 0xa0a0a0);
    drawString(fontRenderer, "memory. If you are playing in a web browser, try", baseX, baseY + 36, 0xa0a0a0);
    drawString(fontRenderer, "downloading the game and playing it offline.", baseX, baseY + 45, 0xa0a0a0);
    drawString(fontRenderer, "To prevent level corruption, the current game has quit.", baseX, baseY + 63, 0xa0a0a0);
    drawString(fontRenderer, "We've tried to free up enough memory to let you go back to", baseX, baseY + 81, 0xa0a0a0);
    drawString(fontRenderer, "the main menu and back to playing, but this may not have worked.", baseX, baseY + 90, 0xa0a0a0);
    drawString(fontRenderer, "Please restart the game if you see this message again.", baseX, baseY + 99, 0xa0a0a0);
    GuiScreen::drawScreen(mouseX, mouseY, partialTick);
}
