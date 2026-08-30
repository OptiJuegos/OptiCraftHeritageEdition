#include "GuiMultiplayer.h"

#include <algorithm>
#include <memory>
#include <sstream>
#include <stdexcept>

#include "ChatAllowedCharacters.h"
#include "CompressedStreamTools.h"
#include "FontRenderer.h"
#include "GuiButton.h"
#if !defined(PS2_PLATFORM) && !defined(WII_PLATFORM)
#include "GuiConnecting.h"
#endif
#include "GuiScreenAddServer.h"
#include "GuiScreenServerList.h"
#include "GuiSlotServer.h"
#include "GuiYesNo.h"
#include "Minecraft.h"
#include "NBTTagCompound.h"
#include "NBTTagList.h"
#include "Packet.h"
#include "RenderEngine.h"
#include "ServerNBTStorage.h"
#include "StatCollector.h"
#include "StringTranslate.h"
#include "java/File.h"
#include "java/JavaNetwork.h"
#include "java/String.h"
#include "pc/lwjgl/Keyboard.h"
#include "platform/Log.h"

std::atomic<int_t> GuiMultiplayer::threadsPending{0};

GuiMultiplayer::GuiMultiplayer(GuiScreen *parent)
    : parentScreen(parent), serverSlotContainer(nullptr), selectedServer(-1),
      buttonEdit(nullptr), buttonSelect(nullptr), buttonDelete(nullptr),
      deleteClicked(false), addClicked(false), editClicked(false), directClicked(false)
{
}

GuiMultiplayer::~GuiMultiplayer()
{
    delete serverSlotContainer;
    serverSlotContainer = nullptr;
}

void GuiMultiplayer::updateScreen()
{
}

void GuiMultiplayer::initGui()
{
#if defined(PS2_PLATFORM) || defined(WII_PLATFORM)
    controlList.clear();
    StringTranslate *translate = StringTranslate::getInstance();
    controlList.push_back(new GuiButton(0, width / 2 - 100, height / 2 + 36,
                                        translate->translateKey("gui.cancel")));
#else
    loadServerList();
    lwjgl::Keyboard::enableRepeatEvents(true);
    controlList.clear();
    delete serverSlotContainer;
    serverSlotContainer = new GuiSlotServer(this);
    initGuiControls();
#endif
}

void GuiMultiplayer::loadServerList()
{
    serverList.clear();
    File *dataDir = Minecraft::getMinecraftDir();
    if (mc == nullptr || dataDir == nullptr)
        return;

    std::unique_ptr<File> file(File::open(*dataDir, "servers.dat"));
    if (!file->exists())
        return;

    try
    {
        std::unique_ptr<std::istream> input(file->toStreamIn());
        std::unique_ptr<NBTTagCompound> root(CompressedStreamTools::readCompound(*input));
        if (root == nullptr || !root->hasKey("servers"))
            return;
        NBTTagList *list = root->getTagList("servers");
        for (int_t i = 0; i < list->tagCount(); ++i)
        {
            NBTTagCompound *tag = dynamic_cast<NBTTagCompound *>(list->tagAt(i));
            if (tag == nullptr)
                continue;
            std::shared_ptr<ServerNBTStorage> server(ServerNBTStorage::createServerNBTStorage(tag));
            if (server != nullptr)
                serverList.push_back(server);
        }
    }
    catch (const std::exception &exception)
    {
        MC_LOG_WARN("network", "Unable to read servers.dat: %s\n", exception.what());
    }
}

void GuiMultiplayer::saveServerList()
{
    File *dataDir = Minecraft::getMinecraftDir();
    if (mc == nullptr || dataDir == nullptr)
        return;

    try
    {
        std::unique_ptr<NBTTagCompound> root(new NBTTagCompound());
        NBTTagList *list = new NBTTagList();
        for (const auto &server : serverList)
        {
            if (server != nullptr)
                list->appendTag(server->getCompoundTag());
        }
        root->setTag("servers", list);

        std::unique_ptr<File> destination(File::open(*dataDir, "servers.dat"));
        std::unique_ptr<File> temporary(File::open(*dataDir, "servers.dat_tmp"));
        if (temporary->exists())
            temporary->remove();
        {
            std::unique_ptr<std::ostream> output(temporary->toStreamOut());
            CompressedStreamTools::writeCompound(root.get(), *output);
            output->flush();
        }
        if (destination->exists() && !destination->remove())
            throw std::runtime_error("Unable to replace servers.dat");
        if (!temporary->renameTo(*destination))
            throw std::runtime_error("Unable to rename servers.dat_tmp");
    }
    catch (const std::exception &exception)
    {
        MC_LOG_WARN("network", "Unable to save servers.dat: %s\n", exception.what());
    }
}

void GuiMultiplayer::initGuiControls()
{
    StringTranslate *translate = StringTranslate::getInstance();
    controlList.push_back(buttonEdit = new GuiButton(7, width / 2 - 154, height - 28, 70, 20,
                                                       translate->translateKey("selectServer.edit")));
    controlList.push_back(buttonDelete = new GuiButton(2, width / 2 - 74, height - 28, 70, 20,
                                                         translate->translateKey("selectServer.delete")));
    controlList.push_back(buttonSelect = new GuiButton(1, width / 2 - 154, height - 52, 100, 20,
                                                         translate->translateKey("selectServer.select")));
    controlList.push_back(new GuiButton(4, width / 2 - 50, height - 52, 100, 20,
                                        translate->translateKey("selectServer.direct")));
    controlList.push_back(new GuiButton(3, width / 2 + 54, height - 52, 100, 20,
                                        translate->translateKey("selectServer.add")));
    controlList.push_back(new GuiButton(8, width / 2 + 4, height - 28, 70, 20,
                                        translate->translateKey("selectServer.refresh")));
    controlList.push_back(new GuiButton(0, width / 2 + 80, height - 28, 75, 20,
                                        translate->translateKey("gui.cancel")));

    const bool valid = selectedServer >= 0 && selectedServer < (int_t)serverList.size();
    buttonSelect->enabled = valid;
    buttonEdit->enabled = valid;
    buttonDelete->enabled = valid;
    if (serverSlotContainer != nullptr)
        serverSlotContainer->registerScrollButtons(controlList, 9, 10);
}

void GuiMultiplayer::onGuiClosed()
{
#if !defined(PS2_PLATFORM) && !defined(WII_PLATFORM)
    lwjgl::Keyboard::enableRepeatEvents(false);
#endif
}

void GuiMultiplayer::actionPerformed(GuiButton *button)
{
    if (button == nullptr || !button->enabled)
        return;

#if defined(PS2_PLATFORM) || defined(WII_PLATFORM)
    if (button->id == 0 && mc != nullptr)
        mc->displayGuiScreen(parentScreen);
    return;
#else
    if (button->id == 2 && selectedServer >= 0 && selectedServer < (int_t)serverList.size())
    {
        const std::string name = serverList[(std::size_t)selectedServer]->name;
        deleteClicked = true;
        StringTranslate *translate = StringTranslate::getInstance();
        mc->displayGuiScreen(new GuiYesNo(this,
            translate->translateKey("selectServer.deleteQuestion"),
            "'" + name + "' " + translate->translateKey("selectServer.deleteWarning"),
            translate->translateKey("selectServer.deleteButton"),
            translate->translateKey("gui.cancel"), selectedServer));
    }
    else if (button->id == 1)
    {
        joinServer(selectedServer);
    }
    else if (button->id == 4)
    {
        directClicked = true;
        tempServer = std::make_shared<ServerNBTStorage>(StatCollector::translateToLocal("selectServer.defaultName"), "");
        mc->displayGuiScreen(new GuiScreenServerList(this, tempServer.get()));
    }
    else if (button->id == 3)
    {
        addClicked = true;
        tempServer = std::make_shared<ServerNBTStorage>(StatCollector::translateToLocal("selectServer.defaultName"), "");
        mc->displayGuiScreen(new GuiScreenAddServer(this, tempServer.get()));
    }
    else if (button->id == 7 && selectedServer >= 0 && selectedServer < (int_t)serverList.size())
    {
        editClicked = true;
        const auto &server = serverList[(std::size_t)selectedServer];
        tempServer = std::make_shared<ServerNBTStorage>(server->name, server->host);
        mc->displayGuiScreen(new GuiScreenAddServer(this, tempServer.get()));
    }
    else if (button->id == 0)
    {
        mc->displayGuiScreen(parentScreen);
    }
    else if (button->id == 8)
    {
        mc->displayGuiScreen(new GuiMultiplayer(parentScreen));
    }
    else if (serverSlotContainer != nullptr)
    {
        serverSlotContainer->actionPerformed(button);
    }
#endif
}

void GuiMultiplayer::confirmClicked(bool confirmed, int_t id)
{
    if (deleteClicked)
    {
        deleteClicked = false;
        if (confirmed && id >= 0 && id < (int_t)serverList.size())
        {
            serverList.erase(serverList.begin() + id);
            if (selectedServer >= (int_t)serverList.size())
                selectedServer = (int_t)serverList.size() - 1;
            saveServerList();
        }
        mc->displayGuiScreen(this);
    }
    else if (directClicked)
    {
        directClicked = false;
        if (confirmed) joinServer(tempServer);
        else mc->displayGuiScreen(this);
    }
    else if (addClicked)
    {
        addClicked = false;
        if (confirmed && tempServer != nullptr)
        {
            serverList.push_back(tempServer);
            saveServerList();
        }
        mc->displayGuiScreen(this);
    }
    else if (editClicked)
    {
        editClicked = false;
        if (confirmed && tempServer != nullptr && selectedServer >= 0 && selectedServer < (int_t)serverList.size())
        {
            serverList[(std::size_t)selectedServer]->name = tempServer->name;
            serverList[(std::size_t)selectedServer]->host = tempServer->host;
            saveServerList();
        }
        mc->displayGuiScreen(this);
    }
    tempServer.reset();
}

int_t GuiMultiplayer::parseIntWithDefault(const std::string &value, int_t defaultValue) const
{
    int_t parsed = 0;
    return String::tryParseInt(String::trimJava(value), parsed) ? parsed : defaultValue;
}

void GuiMultiplayer::splitServerAddress(const std::string &address, std::string &host, int_t &port)
{
    host = address;
    port = 25565;
    if (!address.empty() && address.front() == '[')
    {
        const std::size_t close = address.find(']');
        if (close != std::string::npos && close > 0)
        {
            host = address.substr(1, close - 1);
            std::string rest = String::trimJava(address.substr(close + 1));
            if (!rest.empty() && rest.front() == ':')
            {
                int_t parsed = 0;
                if (String::tryParseInt(String::trimJava(rest.substr(1)), parsed))
                    port = parsed;
            }
            return;
        }
    }
    const std::vector<jstring> parts = String::splitJava(address, ':');
    if (parts.size() == 2)
    {
        host = parts[0];
        int_t parsed = 0;
        if (String::tryParseInt(String::trimJava(parts[1]), parsed))
            port = parsed;
    }
    else if (parts.size() > 2)
    {
        host = address;
    }
}

void GuiMultiplayer::joinServer(int_t index)
{
    if (index >= 0 && index < (int_t)serverList.size())
        joinServer(serverList[(std::size_t)index]);
}

void GuiMultiplayer::joinServer(const std::shared_ptr<ServerNBTStorage> &server)
{
#if defined(PS2_PLATFORM) || defined(WII_PLATFORM)
    (void)server;
#else
    if (server == nullptr || mc == nullptr)
        return;
    std::string host;
    int_t port = 25565;
    splitServerAddress(server->host, host, port);
    mc->displayGuiScreen(new GuiConnecting(mc, host, port));
#endif
}

void GuiMultiplayer::keyTyped(char_t c, int_t)
{
    if (c == '\r' && buttonSelect != nullptr)
        actionPerformed(buttonSelect);
}

void GuiMultiplayer::mouseClicked(int_t x, int_t y, int_t button)
{
    GuiScreen::mouseClicked(x, y, button);
}

void GuiMultiplayer::drawScreen(int_t mouseX, int_t mouseY, float_t partialTick)
{
    lagTooltip.clear();
    StringTranslate *translate = StringTranslate::getInstance();
    drawDefaultBackground();
#if defined(PS2_PLATFORM) || defined(WII_PLATFORM)
    drawCenteredString(fontRenderer, translate->translateKey("multiplayer.title"), width / 2, 20, 0xffffff);
    drawCenteredString(fontRenderer, "Online multiplayer is not available on this platform.",
                       width / 2, height / 2 - 10, 0xa0a0a0);
#else
    if (serverSlotContainer != nullptr)
        serverSlotContainer->drawScreen(mouseX, mouseY, partialTick);
    drawCenteredString(fontRenderer, translate->translateKey("multiplayer.title"), width / 2, 20, 0xffffff);
#endif
    GuiScreen::drawScreen(mouseX, mouseY, partialTick);
    if (!lagTooltip.empty())
        drawTooltip(lagTooltip, mouseX, mouseY);
}

void GuiMultiplayer::drawTooltip(const std::string &text, int_t mouseX, int_t mouseY)
{
    const int_t x = mouseX + 12;
    const int_t y = mouseY - 12;
    const int_t textWidth = fontRenderer->getStringWidth(text);
    drawGradientRect(x - 3, y - 3, x + textWidth + 3, y + 11, 0xc0000000, 0xc0000000);
    fontRenderer->drawStringWithShadow(text, x, y, -1);
}

const std::vector<std::shared_ptr<ServerNBTStorage>> &GuiMultiplayer::getServerList() const { return serverList; }
int_t GuiMultiplayer::getSelectedServer() const { return selectedServer; }
void GuiMultiplayer::setSelectedServer(int_t index) { selectedServer = index; }
GuiButton *GuiMultiplayer::getButtonSelect() const { return buttonSelect; }
GuiButton *GuiMultiplayer::getButtonEdit() const { return buttonEdit; }
GuiButton *GuiMultiplayer::getButtonDelete() const { return buttonDelete; }
void GuiMultiplayer::setTooltipText(const std::string &text) { lagTooltip = text; }
int_t GuiMultiplayer::getThreadsPending() { return threadsPending.load(); }
void GuiMultiplayer::incrementThreadsPending() { threadsPending.fetch_add(1); }
void GuiMultiplayer::decrementThreadsPending() { threadsPending.fetch_sub(1); }

void GuiMultiplayer::pollServer(const std::shared_ptr<ServerNBTStorage> &server)
{
#if defined(PS2_PLATFORM) || defined(WII_PLATFORM)
    if (server != nullptr)
    {
        server->lag = -1;
        server->motd = "\xC2\xA7" "4Can't reach server";
    }
#else
    if (server == nullptr)
        return;

    std::string host;
    int_t port = 25565;
    splitServerAddress(server->host, host, port);
    std::unique_ptr<JavaNetwork::Socket> socket = JavaNetwork::createSocket();
    if (socket == nullptr || !socket->connect(host, port))
        throw std::runtime_error("Can't reach server");

    std::unique_ptr<std::istream> input = JavaNetwork::createInputStream(*socket);
    const char ping = (char)254;
    if (!socket->write(&ping, 1))
        throw std::runtime_error("Failed to send server ping");

    const int packetId = input->get();
    if (packetId != 255)
        throw std::runtime_error("Bad server ping response");

    std::string response = Packet::readString(*input, 256);
    socket->close();

    std::vector<jstring> fields;
    const std::string delimiter = "\xC2\xA7";
    std::size_t fieldStart = 0;
    for (std::size_t separator = response.find(delimiter); separator != std::string::npos;
         separator = response.find(delimiter, fieldStart))
    {
        fields.emplace_back(response.substr(fieldStart, separator - fieldStart));
        fieldStart = separator + delimiter.size();
    }
    fields.emplace_back(response.substr(fieldStart));
    std::string motd = fields.empty() ? response : std::string(fields[0]);
    int_t online = -1;
    int_t maximum = -1;
    if (fields.size() >= 3)
    {
        String::tryParseInt(fields[1], online);
        String::tryParseInt(fields[2], maximum);
    }
    const std::string resolvedMotd = "\xC2\xA7" "7" + motd;
    const std::string resolvedCount = online >= 0 && maximum > 0
        ? "\xC2\xA7" "7" + std::to_string(online) + "\xC2\xA7" "8/" "\xC2\xA7" "7" + std::to_string(maximum)
        : "\xC2\xA7" "8???";
    {
        std::lock_guard<std::mutex> guard(server->stateMutex);
        server->motd = resolvedMotd;
        server->playerCount = resolvedCount;
    }
#endif
}
