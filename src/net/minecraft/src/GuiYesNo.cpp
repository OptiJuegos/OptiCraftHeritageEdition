#include "GuiYesNo.h"
#include "GuiSmallButton.h"
#include "StringTranslate.h"

GuiYesNo::GuiYesNo(GuiScreen *parent, const std::string &msg1, const std::string &msg2, int_t worldNum)
    : GuiYesNo(parent, msg1, msg2,
               StringTranslate::getInstance()->translateKey("gui.yes"),
               StringTranslate::getInstance()->translateKey("gui.no"), worldNum)
{
}

GuiYesNo::GuiYesNo(GuiScreen *parent, const std::string &msg1, const std::string &msg2,
                   const std::string &yes, const std::string &no, int_t worldNum)
    : buttonText1(yes)
    , buttonText2(no)
    , parentScreen(parent)
    , message1(msg1)
    , message2(msg2)
    , worldNumber(worldNum)
{
}

void GuiYesNo::initGui()
{
    controlList.push_back(new GuiSmallButton(0, width / 2 - 155, height / 6 + 96, buttonText1));
    controlList.push_back(new GuiSmallButton(1, width / 2 + 5, height / 6 + 96, buttonText2));
}

void GuiYesNo::actionPerformed(GuiButton *button)
{
    if (parentScreen != nullptr)
        parentScreen->confirmClicked(button->id == 0, worldNumber);
}

void GuiYesNo::drawScreen(int_t mouseX, int_t mouseY, float_t partialTick)
{
    drawDefaultBackground();
    drawCenteredString(fontRenderer, message1, width / 2, 70, 0xffffff);
    drawCenteredString(fontRenderer, message2, width / 2, 90, 0xffffff);
    GuiScreen::drawScreen(mouseX, mouseY, partialTick);
}
