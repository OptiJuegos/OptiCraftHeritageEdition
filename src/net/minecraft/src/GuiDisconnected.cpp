#include "GuiDisconnected.h"

#include "GuiButton.h"
#include "GuiMainMenu.h"
#include "Minecraft.h"
#include "StringTranslate.h"

GuiDisconnected::GuiDisconnected(const std::string &messageKey, const std::string &detailKey,
                                 const std::vector<std::string> &args)
{
    StringTranslate *translate = StringTranslate::getInstance();
    errorMessage = translate->translateKey(messageKey);
    errorDetail = args.empty() ? translate->translateKey(detailKey)
                               : translate->translateKeyFormat(detailKey, args);
}

void GuiDisconnected::updateScreen()
{
}

void GuiDisconnected::keyTyped(char_t, int_t)
{
}

void GuiDisconnected::initGui()
{
    StringTranslate *translate = StringTranslate::getInstance();
    controlList.push_back(new GuiButton(0, width / 2 - 100, height / 4 + 132,
                                        translate->translateKey("gui.toMenu")));
}

void GuiDisconnected::actionPerformed(GuiButton *button)
{
    if (button->id == 0)
        mc->displayGuiScreen(new GuiMainMenu());
}

void GuiDisconnected::drawScreen(int_t mouseX, int_t mouseY, float_t partialTick)
{
    drawDefaultBackground();
    drawCenteredString(fontRenderer, errorMessage, width / 2, height / 2 - 50, 0xffffff);
    drawCenteredString(fontRenderer, errorDetail, width / 2, height / 2 - 10, 0xffffff);
    GuiScreen::drawScreen(mouseX, mouseY, partialTick);
}
