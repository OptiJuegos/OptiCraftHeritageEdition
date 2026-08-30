#include "GuiChat.h"

#include <algorithm>

#include "ChatClickData.h"
#include "EntityClientPlayerMP.h"
#include "EntityPlayerSP.h"
#include "GuiChatConfirmLink.h"
#include "GuiIngame.h"
#include "GuiPlayerInfo.h"
#include "GuiTextField.h"
#include "Minecraft.h"
#include "NetClientHandler.h"
#include "java/String.h"
#include "java/System.h"
#include "pc/lwjgl/Keyboard.h"
#include "pc/lwjgl/Mouse.h"

GuiChat::GuiChat()
    : historyBuffer()
    , sentHistoryCursor(-1)
    , playerNamesFound(false)
    , autocompleteIndex(0)
    , autocompleteNames()
    , pendingUrl()
    , messageField(nullptr)
    , initialMessage()
{
}

GuiChat::GuiChat(const std::string &message)
    : historyBuffer()
    , sentHistoryCursor(-1)
    , playerNamesFound(false)
    , autocompleteIndex(0)
    , autocompleteNames()
    , pendingUrl()
    , messageField(nullptr)
    , initialMessage(message)
{
}

GuiChat::~GuiChat()
{
    delete messageField;
}

void GuiChat::initGui()
{
    lwjgl::Keyboard::enableRepeatEvents(true);
    sentHistoryCursor = (int_t)mc->ingameGUI->getSentMessages().size();
    delete messageField;
#if defined(PS2_PLATFORM) || defined(WII_PLATFORM)
    const int_t fieldY = height - 124;
#else
    const int_t fieldY = height - 12;
#endif
    messageField = new GuiTextField(this, fontRenderer, 4, fieldY, width - 4, 12, "");
    messageField->setMaxStringLength(100);
    messageField->setEnableBackgroundDrawing(false);
    messageField->setFocused(true);
    messageField->setText(initialMessage);
    messageField->setCanLoseFocus(false);
}

void GuiChat::onGuiClosed()
{
    lwjgl::Keyboard::enableRepeatEvents(false);
    mc->ingameGUI->resetChatScroll();
    if (messageField != nullptr)
        messageField->setFocused(false);
}

void GuiChat::updateScreen()
{
    if (messageField != nullptr)
        messageField->updateCursorCounter();
}

void GuiChat::keyTyped(char_t c, int_t key)
{
    if (key == lwjgl::Keyboard::KEY_TAB)
        completePlayerName();
    else
        playerNamesFound = false;

    if (key == lwjgl::Keyboard::KEY_ESCAPE)
    {
        mc->displayGuiScreen(nullptr);
        return;
    }
    if (key == lwjgl::Keyboard::KEY_RETURN)
    {
        std::string message = String::trimJava(messageField != nullptr ? messageField->getText() : "");
        if (!message.empty() && !mc->lineIsCommand(message))
            mc->thePlayer->sendChatMessage(message);
        mc->displayGuiScreen(nullptr);
        return;
    }
    if (key == lwjgl::Keyboard::KEY_UP)
    {
        getSentHistory(-1);
        return;
    }
    if (key == lwjgl::Keyboard::KEY_DOWN)
    {
        getSentHistory(1);
        return;
    }
    if (key == lwjgl::Keyboard::KEY_PRIOR)
    {
        mc->ingameGUI->scrollChat(19);
        return;
    }
    if (key == lwjgl::Keyboard::KEY_NEXT)
    {
        mc->ingameGUI->scrollChat(-19);
        return;
    }
    if (messageField != nullptr)
        messageField->textboxKeyTyped(c, key);
}

void GuiChat::handleMouseInput()
{
    GuiScreen::handleMouseInput();
    int_t wheel = lwjgl::Mouse::getEventDWheel();
    if (wheel == 0)
        return;
    if (wheel > 1) wheel = 1;
    if (wheel < -1) wheel = -1;
    if (!GuiScreen::isShiftKeyDown())
        wheel *= 7;
    mc->ingameGUI->scrollChat(wheel);
}

void GuiChat::mouseClicked(int_t x, int_t y, int_t button)
{
    if (button == 0)
    {
        ChatClickData *click = mc->ingameGUI->getChatClickData(lwjgl::Mouse::getX(), lwjgl::Mouse::getY());
        if (click != nullptr)
        {
            std::string url = click->getUrl();
            if (!url.empty())
            {
                pendingUrl = url;
                mc->displayGuiScreen(new GuiChatConfirmLink(this, this, click->getClickedText(), 0, click));
                return;
            }
            delete click;
        }
    }

    if (messageField != nullptr)
        messageField->mouseClicked(x, y, button);
    GuiScreen::mouseClicked(x, y, button);
}

void GuiChat::confirmClicked(bool confirmed, int_t id)
{
    if (id != 0)
        return;
    if (confirmed && !pendingUrl.empty())
        System::openURL(pendingUrl);
    pendingUrl.clear();
    mc->displayGuiScreen(this);
}

void GuiChat::completePlayerName()
{
    if (messageField == nullptr)
        return;

    if (playerNamesFound)
    {
        messageField->deleteWords(-1);
        if (autocompleteIndex >= (int_t)autocompleteNames.size())
            autocompleteIndex = 0;
    }
    else
    {
        int_t wordStart = messageField->getNthWordFromCursor(-1);
        if (messageField->getCursorPosition() - wordStart < 1)
            return;

        autocompleteNames.clear();
        std::string typed = String::substringUtf16(
            messageField->getText(), wordStart, String::utf16Length(messageField->getText()));
        std::string lower = String::toLowerCaseJava(typed);
        EntityClientPlayerMP *player = dynamic_cast<EntityClientPlayerMP *>(mc->thePlayer);
        if (player == nullptr || player->sendQueue == nullptr)
            return;

        for (GuiPlayerInfo *info : player->sendQueue->getPlayerNames())
        {
            if (info != nullptr && info->nameStartsWith(lower))
                autocompleteNames.push_back(info->name);
        }
        if (autocompleteNames.empty())
            return;

        playerNamesFound = true;
        autocompleteIndex = 0;
        messageField->deleteFromCursor(wordStart - messageField->getCursorPosition());
    }

    if (autocompleteNames.size() > 1)
    {
        std::string names;
        for (const std::string &name : autocompleteNames)
        {
            if (!names.empty()) names += ", ";
            names += name;
        }
        mc->ingameGUI->addChatMessage(names);
    }

    messageField->writeText(autocompleteNames[(std::size_t)autocompleteIndex++]);
}

void GuiChat::getSentHistory(int_t direction)
{
    if (messageField == nullptr)
        return;
    const std::vector<std::string> &history = mc->ingameGUI->getSentMessages();
    int_t next = sentHistoryCursor + direction;
    next = std::max(0, std::min(next, (int_t)history.size()));
    if (next == sentHistoryCursor)
        return;

    if (next == (int_t)history.size())
    {
        sentHistoryCursor = next;
        messageField->setText(historyBuffer);
    }
    else
    {
        if (sentHistoryCursor == (int_t)history.size())
            historyBuffer = messageField->getText();
        messageField->setText(history[(std::size_t)next]);
        sentHistoryCursor = next;
    }
}

void GuiChat::drawScreen(int_t mouseX, int_t mouseY, float_t partialTick)
{
#if defined(PS2_PLATFORM) || defined(WII_PLATFORM)
    const int_t fieldY = height - 124;
#else
    const int_t fieldY = height - 14;
#endif
    drawRect(2, fieldY, width - 2, fieldY + 12, 0x80000000);
    if (messageField != nullptr)
        messageField->drawTextBox();
    GuiScreen::drawScreen(mouseX, mouseY, partialTick);
}

std::string GuiChat::getMessage() const
{
    if (messageField != nullptr)
        return messageField->getText();

    return std::string();
}

void GuiChat::setMessage(const std::string &message)
{
    if (messageField != nullptr)
        messageField->setText(message);
    else
        initialMessage = message;
}
