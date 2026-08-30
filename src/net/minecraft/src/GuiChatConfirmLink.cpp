#include "GuiChatConfirmLink.h"

#include "ChatClickData.h"
#include "GuiScreen.h"

GuiChatConfirmLink::GuiChatConfirmLink(GuiChat *chatIn, GuiScreen *parent, const std::string &link,
                                       int_t id, ChatClickData *clickDataIn)
    : GuiConfirmOpenLink(parent, link, id)
    , chat(chatIn)
    , clickData(clickDataIn)
{
}


GuiChatConfirmLink::~GuiChatConfirmLink()
{
    delete clickData;
    clickData = nullptr;
}

void GuiChatConfirmLink::copyLinkToClipboard()
{
    if (clickData != nullptr)
        GuiScreen::setClipboardString(clickData->getClickedText());
}
