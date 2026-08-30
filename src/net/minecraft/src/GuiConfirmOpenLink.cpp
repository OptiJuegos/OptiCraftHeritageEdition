#include "GuiConfirmOpenLink.h"

#include "GuiButton.h"
#include "StringTranslate.h"

GuiConfirmOpenLink::GuiConfirmOpenLink(GuiScreen *parent, const std::string &link, int_t id)
    : GuiYesNo(parent, StringTranslate::getInstance()->translateKey("chat.link.confirm"), link, id)
{
    StringTranslate *translate = StringTranslate::getInstance();
    buttonText1 = translate->translateKey("gui.yes");
    buttonText2 = translate->translateKey("gui.no");
    copyText = translate->translateKey("chat.copy");
    warningText = translate->translateKey("chat.link.warning");
}

void GuiConfirmOpenLink::initGui()
{
    controlList.push_back(new GuiButton(0, width / 3 - 83, height / 6 + 96, 100, 20, buttonText1));
    controlList.push_back(new GuiButton(2, width / 3 + 22, height / 6 + 96, 100, 20, copyText));
    controlList.push_back(new GuiButton(1, width / 3 + 127, height / 6 + 96, 100, 20, buttonText2));
}

void GuiConfirmOpenLink::actionPerformed(GuiButton *button)
{
    if (button->id == 2)
    {
        copyLinkToClipboard();
        GuiYesNo::actionPerformed(button);
        return;
    }
    GuiYesNo::actionPerformed(button);
}

void GuiConfirmOpenLink::drawScreen(int_t mouseX, int_t mouseY, float_t partialTick)
{
    GuiYesNo::drawScreen(mouseX, mouseY, partialTick);
    drawCenteredString(fontRenderer, warningText, width / 2, 110, 0xffcccc);
}
