#include "GuiScreenServerList.h"

#include "GuiButton.h"
#include "GuiTextField.h"
#include "GuiTextFieldSelector.h"
#include "ServerNBTStorage.h"
#include "StringTranslate.h"
#include "java/String.h"
#include "pc/lwjgl/Keyboard.h"

jstring GuiScreenServerList::lastAddress;

GuiScreenServerList::GuiScreenServerList(GuiScreen *parent, ServerNBTStorage *server)
    : parentGui(parent), serverListStorage(server), serverTextField(nullptr), buttonSelect(nullptr)
{
}

GuiScreenServerList::~GuiScreenServerList()
{
    delete serverTextField;
}

void GuiScreenServerList::updateScreen()
{
    if (serverTextField != nullptr) serverTextField->updateCursorCounter();
}

void GuiScreenServerList::initGui()
{
#if !defined(PS2_PLATFORM) && !defined(WII_PLATFORM)
    lwjgl::Keyboard::enableRepeatEvents(true);
#endif
    StringTranslate *translate = StringTranslate::getInstance();
    controlList.clear();
    delete serverTextField;
    serverTextField = new GuiTextField(this, fontRenderer, width / 2 - 100, 116, 200, 20, lastAddress);
    serverTextField->setMaxStringLength(128);
    serverTextField->setFocused(true);
    controlList.push_back(new GuiTextFieldSelector(2, width / 2 - 100, 116, 200, 20));
    controlList.push_back(buttonSelect = new GuiButton(0, width / 2 - 100, height / 4 + 108,
                                                       translate->translateKey("selectServer.select")));
    controlList.push_back(new GuiButton(1, width / 2 - 100, height / 4 + 132,
                                        translate->translateKey("gui.cancel")));
    updateSelectButtonState();
}

void GuiScreenServerList::onGuiClosed()
{
#if !defined(PS2_PLATFORM) && !defined(WII_PLATFORM)
    lwjgl::Keyboard::enableRepeatEvents(false);
#endif
    if (serverTextField != nullptr) lastAddress = serverTextField->getText();
    if (serverTextField != nullptr) serverTextField->setFocused(false);
}

void GuiScreenServerList::actionPerformed(GuiButton *button)
{
    if (button == nullptr || !button->enabled)
        return;
    if (button->id == 2)
    {
        if (serverTextField != nullptr) serverTextField->setFocused(true);
    }
    else if (button->id == 1)
    {
        if (parentGui != nullptr) parentGui->confirmClicked(false, 0);
    }
    else if (button->id == 0)
    {
        if (serverListStorage != nullptr && serverTextField != nullptr)
            serverListStorage->host = serverTextField->getText();
        if (parentGui != nullptr) parentGui->confirmClicked(true, 0);
    }
}

void GuiScreenServerList::keyTyped(char_t c, int_t key)
{
    if (serverTextField != nullptr) serverTextField->textboxKeyTyped(c, key);
    if ((c == '\r' || key == lwjgl::Keyboard::KEY_RETURN) && buttonSelect != nullptr)
        actionPerformed(buttonSelect);
    else if (key == lwjgl::Keyboard::KEY_ESCAPE && parentGui != nullptr)
    {
        parentGui->confirmClicked(false, 0);
        return;
    }
    updateSelectButtonState();
}

void GuiScreenServerList::mouseClicked(int_t x, int_t y, int_t button)
{
    GuiScreen::mouseClicked(x, y, button);
    if (serverTextField != nullptr) serverTextField->mouseClicked(x, y, button);
}

void GuiScreenServerList::updateSelectButtonState()
{
    if (buttonSelect == nullptr || serverTextField == nullptr)
        return;
    const std::string address = serverTextField->getText();
    bool valid = !address.empty();
    if (valid && address.front() != '[')
    {
        const std::vector<jstring> parts = String::splitJava(address, ':');
        if (parts.size() > 2)
            valid = false;
    }
    buttonSelect->enabled = valid;
}

void GuiScreenServerList::drawScreen(int_t mouseX, int_t mouseY, float_t partialTick)
{
    StringTranslate *translate = StringTranslate::getInstance();
    drawDefaultBackground();
    drawCenteredString(fontRenderer, translate->translateKey("selectServer.direct"), width / 2, height / 4 - 40, 0xffffff);
    drawString(fontRenderer, translate->translateKey("addServer.enterIp"), width / 2 - 100, 104, 0xa0a0a0);
    if (serverTextField != nullptr) serverTextField->drawTextBox();
    GuiScreen::drawScreen(mouseX, mouseY, partialTick);
}
