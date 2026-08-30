#pragma once

#include "GuiConfirmOpenLink.h"

class ChatClickData;
class GuiChat;

// net.minecraft.src.GuiChatConfirmLink
class GuiChatConfirmLink : public GuiConfirmOpenLink
{
public:
    GuiChatConfirmLink(GuiChat *chat, GuiScreen *parent, const std::string &link, int_t id,
                       ChatClickData *clickData);
    ~GuiChatConfirmLink() override;

protected:
    void copyLinkToClipboard() override;

private:
    GuiChat *chat;
    ChatClickData *clickData;
};
