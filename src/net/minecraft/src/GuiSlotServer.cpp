#include "GuiSlotServer.h"

#include <memory>

#include "FontRenderer.h"
#include "GuiButton.h"
#include "GuiMultiplayer.h"
#include "Minecraft.h"
#include "RenderEngine.h"
#include "ServerNBTStorage.h"
#include "java/System.h"
#include "ThreadPollServers.h"

GuiSlotServer::GuiSlotServer(GuiMultiplayer *parent)
    : GuiSlot(parent->getMinecraft(), parent->width, parent->height, 32, parent->height - 64, 36),
      parentGui(parent)
{
}

int_t GuiSlotServer::getSize()
{
    return parentGui != nullptr ? (int_t)parentGui->getServerList().size() : 0;
}

void GuiSlotServer::elementClicked(int_t index, bool doubleClicked)
{
    if (parentGui == nullptr)
        return;
    parentGui->setSelectedServer(index);
    const bool valid = index >= 0 && index < getSize();
    if (doubleClicked && valid)
        parentGui->joinServer(index);
}

bool GuiSlotServer::isSelected(int_t index)
{
    return parentGui != nullptr && index == parentGui->getSelectedServer();
}

int_t GuiSlotServer::getContentHeight()
{
    return getSize() * 36;
}

void GuiSlotServer::drawBackground()
{
    if (parentGui != nullptr)
        parentGui->drawDefaultBackground();
}

void GuiSlotServer::drawSlot(int_t index, int_t x, int_t y, int_t, Tessellator *)
{
    if (parentGui == nullptr || index < 0 || index >= getSize())
        return;

    const std::shared_ptr<ServerNBTStorage> server = parentGui->getServerList()[(std::size_t)index];
    if (server == nullptr)
        return;

    std::string name;
    std::string host;
    std::string motd;
    std::string playerCount;
    long_t lag = -2;
    bool polled = false;
    bool shouldPoll = false;
    {
        std::lock_guard<std::mutex> guard(server->stateMutex);
        name = server->name;
        host = server->host;
        motd = server->motd;
        playerCount = server->playerCount;
        lag = server->lag;
        polled = server->polled;
#ifdef PS2_PLATFORM
        constexpr int_t maxPollThreads = 1;
#else
        constexpr int_t maxPollThreads = 5;
#endif
        if (!server->polled && GuiMultiplayer::getThreadsPending() < maxPollThreads)
        {
            server->polled = true;
            server->lag = -2;
            server->motd.clear();
            server->playerCount.clear();
            polled = true;
            lag = -2;
            motd.clear();
            playerCount.clear();
            shouldPoll = true;
        }
    }

    if (shouldPoll)
    {
        GuiMultiplayer::incrementThreadsPending();
        ThreadPollServers::start(server);
    }

    FontRenderer *font = parentGui->getFontRenderer();
    parentGui->drawString(font, name, x + 2, y + 1, 0xffffff);
    parentGui->drawString(font, motd, x + 2, y + 12, 0x808080);
    parentGui->drawString(font, playerCount, x + 215 - font->getStringWidth(playerCount), y + 12, 0x808080);
    parentGui->drawString(font, host, x + 2, y + 23, 0x303030);

    Minecraft *minecraft = parentGui->getMinecraft();
    if (minecraft == nullptr || minecraft->renderEngine == nullptr)
        return;
    minecraft->renderEngine->bindTexture(minecraft->renderEngine->getTexture("/gui/icons.png"));

    int_t iconColumn = 0;
    int_t iconRow = 0;
    std::string tooltip;
    if (polled && lag != -2)
    {
        if (lag < 0) iconRow = 5;
        else if (lag < 150) iconRow = 0;
        else if (lag < 300) iconRow = 1;
        else if (lag < 600) iconRow = 2;
        else if (lag < 1000) iconRow = 3;
        else iconRow = 4;
        tooltip = lag < 0 ? "(no connection)" : std::to_string(lag) + "ms";
    }
    else
    {
        iconColumn = 1;
        iconRow = (int_t)((System::currentTimeMillis() / 100 + index * 2) & 7);
        if (iconRow > 4) iconRow = 8 - iconRow;
        tooltip = "Polling..";
    }

    parentGui->drawTexturedModalRect(x + 205, y, iconColumn * 10, 176 + iconRow * 8, 10, 8);
    constexpr int_t padding = 4;
    if (currentMouseX >= x + 205 - padding && currentMouseY >= y - padding &&
        currentMouseX <= x + 215 + padding && currentMouseY <= y + 8 + padding)
    {
        parentGui->setTooltipText(tooltip);
    }
}
