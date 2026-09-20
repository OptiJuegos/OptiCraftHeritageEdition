#include "GuiScreenAddServer.h"

#include "GuiButton.h"
#include "GuiTextField.h"
#include "GuiTextFieldSelector.h"
#include "ServerNBTStorage.h"
#include "StringTranslate.h"
#include "java/String.h"
#include "pc/lwjgl/Keyboard.h"

GuiScreenAddServer::GuiScreenAddServer(GuiScreen *parent, ServerNBTStorage *server)
    : parentGui(parent), serverAddress(nullptr), serverName(nullptr), buttonAdd(nullptr), serverNBTStorage(server)
{
}

GuiScreenAddServer::~GuiScreenAddServer()
{
    delete serverAddress;
    delete serverName;
}

void GuiScreenAddServer::updateScreen()
{
    if (serverName != nullptr) serverName->updateCursorCounter();
    if (serverAddress != nullptr) serverAddress->updateCursorCounter();
}

void GuiScreenAddServer::initGui()
{
#if !defined(PS2_PLATFORM) && !defined(WII_PLATFORM)
    lwjgl::Keyboard::enableRepeatEvents(true);
#endif
    StringTranslate *translate = StringTranslate::getInstance();
    controlList.clear();
    delete serverName;
    delete serverAddress;
    serverName = new GuiTextField(this, fontRenderer, width / 2 - 100, 76, 200, 20,
                                  serverNBTStorage != nullptr ? serverNBTStorage->name : "");
    serverName->setFocused(true);
    serverAddress = new GuiTextField(this, fontRenderer, width / 2 - 100, 116, 200, 20,
                                     serverNBTStorage != nullptr ? serverNBTStorage->host : "");
    serverAddress->setMaxStringLength(128);

    // Keep fields and buttons in their visual top-to-bottom order so D-pad
    // focus follows the screen instead of skipping directly to Add/Cancel.
    controlList.push_back(new GuiTextFieldSelector(10, width / 2 - 100, 76, 200, 20));
    controlList.push_back(new GuiTextFieldSelector(11, width / 2 - 100, 116, 200, 20));
    controlList.push_back(buttonAdd = new GuiButton(0, width / 2 - 100, height / 4 + 108,
                                                    translate->translateKey("addServer.add")));
    controlList.push_back(new GuiButton(1, width / 2 - 100, height / 4 + 132,
                                        translate->translateKey("gui.cancel")));
    updateAddButtonState();
}

void GuiScreenAddServer::onGuiClosed()
{
#if !defined(PS2_PLATFORM) && !defined(WII_PLATFORM)
    lwjgl::Keyboard::enableRepeatEvents(false);
#endif
    if (serverName != nullptr) serverName->setFocused(false);
    if (serverAddress != nullptr) serverAddress->setFocused(false);
}

void GuiScreenAddServer::actionPerformed(GuiButton *button)
{
    if (button == nullptr || !button->enabled)
        return;
    if (button->id == 10)
    {
        if (serverName != nullptr) serverName->setFocused(true);
    }
    else if (button->id == 11)
    {
        if (serverAddress != nullptr) serverAddress->setFocused(true);
    }
    else if (button->id == 1)
    {
        if (parentGui != nullptr) parentGui->confirmClicked(false, 0);
    }
    else if (button->id == 0)
    {
        if (serverNBTStorage != nullptr)
        {
            serverNBTStorage->name = serverName != nullptr ? serverName->getText() : "";
            serverNBTStorage->host = serverAddress != nullptr ? serverAddress->getText() : "";
        }
        if (parentGui != nullptr) parentGui->confirmClicked(true, 0);
    }
}

void GuiScreenAddServer::keyTyped(char_t c, int_t key)
{
#if defined(PS2_PLATFORM) || defined(WII_PLATFORM)
    if (c == '\r' || key == lwjgl::Keyboard::KEY_RETURN)
    {
        if (serverName != nullptr && serverName->getFocused())
        {
            serverName->setFocused(false);
            if (serverAddress != nullptr) serverAddress->setFocused(true);
            updateAddButtonState();
            return;
        }
        if (serverAddress != nullptr && serverAddress->getFocused())
        {
            updateAddButtonState();
            if (buttonAdd != nullptr && buttonAdd->enabled)
                actionPerformed(buttonAdd);
            return;
        }
    }
#endif
    if (key == lwjgl::Keyboard::KEY_ESCAPE)
    {
        if (parentGui != nullptr) parentGui->confirmClicked(false, 0);
        return;
    }
    if (c == '\t')
    {
        const bool nameFocused = serverName != nullptr && serverName->isFocused;
        if (serverName != nullptr) serverName->setFocused(!nameFocused);
        if (serverAddress != nullptr) serverAddress->setFocused(nameFocused);
    }
    else
    {
        if (serverName != nullptr) serverName->textboxKeyTyped(c, key);
        if (serverAddress != nullptr) serverAddress->textboxKeyTyped(c, key);
    }
#if !defined(PS2_PLATFORM) && !defined(WII_PLATFORM)
    if ((c == '\r' || key == 28) && !controlList.empty())
        actionPerformed(buttonAdd);
#endif
    updateAddButtonState();
}

void GuiScreenAddServer::mouseClicked(int_t x, int_t y, int_t button)
{
    GuiScreen::mouseClicked(x, y, button);
    if (serverAddress != nullptr) serverAddress->mouseClicked(x, y, button);
    if (serverName != nullptr) serverName->mouseClicked(x, y, button);
}

void GuiScreenAddServer::updateAddButtonState()
{
    if (buttonAdd == nullptr || serverAddress == nullptr || serverName == nullptr)
        return;
    const std::string address = serverAddress->getText();
    bool valid = !address.empty() && !serverName->getText().empty();
    if (valid && address.front() != '[')
    {
        const std::vector<jstring> parts = String::splitJava(address, ':');
        if (parts.size() > 2)
            valid = false;
    }
    buttonAdd->enabled = valid;
}

void GuiScreenAddServer::drawScreen(int_t mouseX, int_t mouseY, float_t partialTick)
{
    StringTranslate *translate = StringTranslate::getInstance();
    drawDefaultBackground();
    drawCenteredString(fontRenderer, translate->translateKey("addServer.title"), width / 2, height / 4 - 40, 0xffffff);
    drawString(fontRenderer, translate->translateKey("addServer.enterName"), width / 2 - 100, 63, 0xa0a0a0);
    drawString(fontRenderer, translate->translateKey("addServer.enterIp"), width / 2 - 100, 104, 0xa0a0a0);
    if (serverName != nullptr) serverName->drawTextBox();
    if (serverAddress != nullptr) serverAddress->drawTextBox();
    GuiScreen::drawScreen(mouseX, mouseY, partialTick);
}
