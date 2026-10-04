#include "net/minecraft/src/UiStrings.h"
#include "GuiMultiplayer.h"

#include <algorithm>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <vector>

#include "ChatAllowedCharacters.h"
#include "CompressedStreamTools.h"
#include "FontRenderer.h"
#include "GuiButton.h"
#ifndef NO_NETWORK
#include "GuiConnecting.h"
#endif
#include "GuiScreenAddServer.h"
#include "GuiScreenServerList.h"
#include "GuiSlotServer.h"
#include "GuiYesNo.h"
#include "Minecraft.h"
#include "SoundManager.h"
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
#include "platform/Input.h"
#include "platform/Log.h"
#include "platform/Storage.h"

namespace
{
constexpr int_t SERVER_LIST_FOCUS = -1;
constexpr int_t TOP_BUTTONS[] = {1, 4, 3};
constexpr int_t BOTTOM_BUTTONS[] = {7, 2, 8, 0};
constexpr std::size_t MAX_SERVER_LIST_BYTES = 1024 * 1024;
}

std::atomic<int_t> GuiMultiplayer::threadsPending{0};

GuiMultiplayer::GuiMultiplayer(GuiScreen *parent)
    : parentScreen(parent), serverSlotContainer(nullptr), selectedServer(-1),
      buttonEdit(nullptr), buttonSelect(nullptr), buttonDelete(nullptr),
      deleteClicked(false), addClicked(false), editClicked(false), directClicked(false),
      controllerFocus(SERVER_LIST_FOCUS), padOpeningTurnArmed(false)
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
#ifdef NO_NETWORK
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
#if (defined(PS2_PLATFORM) || defined(CTR_PLATFORM)) && !defined(NO_NETWORK)
    // Pad-driven consoles need a selection to exist before any button is
    // pressed: the first server starts out highlighted and the focus sits
    // on the list, so the very first D-pad press already does something.
    if (serverList.empty())
        setSelectedServer(-1);
    else if (selectedServer < 0 || selectedServer >= static_cast<int_t>(serverList.size()))
        setSelectedServer(0);
    controllerFocus = serverList.empty() ? 4 : SERVER_LIST_FOCUS;
    syncControllerFocus();
    // initGui runs again on every displayGuiScreen -- opening, Refresh's new
    // instance, and the same-instance returns from Add/Edit/Delete confirm --
    // so arming here catches every (re)entry. See handleSpecializedMenuInput.
    padOpeningTurnArmed = true;
#endif
#endif
}

namespace
{
// File-level halves of loadServerList()/saveServerList(), free of the GUI
// instance so the 3DS QR flow can also store a scanned server address
// (GuiMultiplayer::addServerAndSave below).
bool readServerListFile(std::vector<std::shared_ptr<ServerNBTStorage>> &out)
{
    out.clear();
    File *dataDir = Minecraft::getMinecraftDir();
    if (dataDir == nullptr)
        return false;
    const std::string path = PlatformStorage::join(dataDir->toString(), "servers.dat");
    if (!PlatformStorage::exists(path))
        return true; // no list yet is a valid, empty list
    const std::int64_t fileSize = PlatformStorage::getFileSize(path);
    if (fileSize == 0 || fileSize > static_cast<std::int64_t>(MAX_SERVER_LIST_BYTES))
    {
        MC_LOG_WARN("network", "Refusing invalid servers.dat size: %lld bytes\n",
                    static_cast<long long>(fileSize));
        return false;
    }

    try
    {
        std::vector<unsigned char> bytes;
        if (!PlatformStorage::readFile(path, bytes))
            throw std::runtime_error("Unable to read servers.dat from storage");
        if (bytes.empty() || bytes.size() > MAX_SERVER_LIST_BYTES)
            throw std::runtime_error("Invalid servers.dat payload size");

        const std::string payload(reinterpret_cast<const char *>(bytes.data()), bytes.size());
        std::istringstream input(payload, std::ios::in | std::ios::binary);
        std::unique_ptr<NBTTagCompound> root(CompressedStreamTools::readCompound(input));
        if (root == nullptr || !root->hasKey("servers"))
            return true;
        NBTTagList *list = root->getTagList("servers");
        for (int_t i = 0; i < list->tagCount(); ++i)
        {
            NBTTagCompound *tag = dynamic_cast<NBTTagCompound *>(list->tagAt(i));
            if (tag == nullptr)
                continue;
            std::shared_ptr<ServerNBTStorage> server(ServerNBTStorage::createServerNBTStorage(tag));
            if (server != nullptr)
                out.push_back(server);
        }
    }
    catch (const std::exception &exception)
    {
        MC_LOG_WARN("network", "Unable to read servers.dat: %s\n", exception.what());
        return false;
    }
    return true;
}

bool writeServerListFile(const std::vector<std::shared_ptr<ServerNBTStorage>> &list)
{
    File *dataDir = Minecraft::getMinecraftDir();
    if (dataDir == nullptr)
        return false;

    try
    {
        std::unique_ptr<NBTTagCompound> root(new NBTTagCompound());
        NBTTagList *tagList = new NBTTagList();
        for (const auto &server : list)
        {
            if (server != nullptr)
                tagList->appendTag(server->getCompoundTag());
        }
        root->setTag("servers", tagList);

        std::ostringstream output(std::ios::out | std::ios::binary);
        CompressedStreamTools::writeCompound(root.get(), output);
        if (!output.good())
            throw std::runtime_error("Unable to serialize servers.dat");
        const std::string payload = output.str();
        if (payload.empty() || payload.size() > MAX_SERVER_LIST_BYTES)
            throw std::runtime_error("Invalid serialized servers.dat size");

        const std::string directory = dataDir->toString();
        const std::string destination = PlatformStorage::join(directory, "servers.dat");
        if (!PlatformStorage::mkdirs(directory))
            throw std::runtime_error("Unable to create server-list directory");

        bool saved = false;
        if (PlatformStorage::supportsAtomicRename())
        {
            const std::string temporary = PlatformStorage::join(directory, "servers.dat_tmp");
            PlatformStorage::removeFile(temporary);
            if (PlatformStorage::writeFile(temporary, payload.data(), payload.size()))
            {
                if (PlatformStorage::exists(destination) && !PlatformStorage::removeFile(destination))
                    throw std::runtime_error("Unable to replace servers.dat");
                saved = PlatformStorage::renameFile(temporary, destination);
                if (!saved)
                    PlatformStorage::removeFile(temporary);
            }
        }
        else
        {
            saved = PlatformStorage::writeFile(destination, payload.data(), payload.size());
        }

        if (!saved)
            throw std::runtime_error("Unable to write servers.dat");
    }
    catch (const std::exception &exception)
    {
        MC_LOG_WARN("network", "Unable to save servers.dat: %s\n", exception.what());
        return false;
    }
    return true;
}
} // namespace

void GuiMultiplayer::loadServerList()
{
    if (mc == nullptr)
        return;
    std::vector<std::shared_ptr<ServerNBTStorage>> loaded;
    if (readServerListFile(loaded))
        serverList = loaded;
    else
        serverList.clear();
}

void GuiMultiplayer::saveServerList()
{
    if (mc == nullptr)
        return;
    writeServerListFile(serverList);
}

#ifdef CTR_PLATFORM
bool GuiMultiplayer::addServerAndSave(const std::string &name, const std::string &host,
                                      std::string &outError)
{
    std::vector<std::shared_ptr<ServerNBTStorage>> stored;
    if (!readServerListFile(stored))
    {
        outError = "The server list could not be read";
        return false;
    }
    for (const auto &server : stored)
    {
        if (server != nullptr && server->host == host)
        {
            outError.clear(); // already stored: nothing to add, nothing to fail
            return true;
        }
    }
    stored.push_back(std::make_shared<ServerNBTStorage>(name, host));
    if (!writeServerListFile(stored))
    {
        outError = "The server list could not be saved";
        return false;
    }
    return true;
}
#endif

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

    if (serverSlotContainer != nullptr)
        serverSlotContainer->registerScrollButtons(controlList, 9, 10);
    updateSelectionButtons();
}

void GuiMultiplayer::onGuiClosed()
{
#ifndef NO_NETWORK
    lwjgl::Keyboard::enableRepeatEvents(false);
#endif
}

void GuiMultiplayer::actionPerformed(GuiButton *button)
{
    if (button == nullptr || !button->enabled)
        return;

#ifdef NO_NETWORK
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
#ifdef NO_NETWORK
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

bool GuiMultiplayer::usesSpecializedMenuNavigation() const
{
    // The server list is a touch-only GuiSlot in the shared design, so the
    // pad-driven consoles take the screen over with their own focus model
    // (see handleSpecializedMenuInput). The 3DS joins the PS2 here: without
    // this, the D-pad only reached the row of buttons and the entries
    // themselves required the touch panel.
#if (defined(PS2_PLATFORM) || defined(CTR_PLATFORM)) && !defined(NO_NETWORK)
    return true;
#else
    return false;
#endif
}

bool GuiMultiplayer::suppressesPlatformPointerInput() const
{
#if defined(PS2_PLATFORM) && !defined(NO_NETWORK)
    return true;
#else
    // Deliberately NOT set on the 3DS: the touch panel is the screen's other
    // input, and it must keep working alongside the pad navigation (the PS2
    // suppresses its pointer because the D-pad is all it has).
    return false;
#endif
}

GuiButton *GuiMultiplayer::findButton(int_t buttonId) const
{
    for (GuiButton *button : controlList)
    {
        if (button != nullptr && button->id == buttonId)
            return button;
    }
    return nullptr;
}

void GuiMultiplayer::updateSelectionButtons()
{
    const bool valid = selectedServer >= 0 && selectedServer < static_cast<int_t>(serverList.size());
    if (buttonSelect != nullptr) buttonSelect->enabled = valid;
    if (buttonEdit != nullptr) buttonEdit->enabled = valid;
    if (buttonDelete != nullptr) buttonDelete->enabled = valid;
}

void GuiMultiplayer::syncControllerFocus()
{
#if defined(PS2_PLATFORM) || (defined(CTR_PLATFORM) && !defined(NO_NETWORK))
    for (GuiButton *button : controlList)
    {
        if (button != nullptr)
            button->setKeyboardSelected(false);
    }
    if (controllerFocus == SERVER_LIST_FOCUS)
        return;
    GuiButton *button = findButton(controllerFocus);
    if (button != nullptr && button->enabled && button->enabled2)
        button->setKeyboardSelected(true);
#endif
}

void GuiMultiplayer::focusButton(int_t buttonId)
{
#if defined(PS2_PLATFORM) || (defined(CTR_PLATFORM) && !defined(NO_NETWORK))
    GuiButton *button = findButton(buttonId);
    if (button == nullptr || !button->enabled || !button->enabled2)
        return;
    if (controllerFocus != buttonId && mc != nullptr && mc->sndManager != nullptr)
        mc->sndManager->playSoundFX("random.focus", 1.0f, 1.0f);
    controllerFocus = buttonId;
    syncControllerFocus();
#else
    (void)buttonId;
#endif
}

void GuiMultiplayer::moveControllerFocusHorizontal(int_t direction)
{
#if defined(PS2_PLATFORM) || (defined(CTR_PLATFORM) && !defined(NO_NETWORK))
    if (direction == 0)
        return;
    if (controllerFocus == SERVER_LIST_FOCUS)
    {
        if (direction > 0)
        {
            for (int_t id : TOP_BUTTONS)
            {
                GuiButton *button = findButton(id);
                if (button != nullptr && button->enabled && button->enabled2)
                {
                    focusButton(id);
                    return;
                }
            }
        }
        return;
    }

    const int_t *row = nullptr;
    int_t rowSize = 0;
    for (int_t id : TOP_BUTTONS)
        if (id == controllerFocus) { row = TOP_BUTTONS; rowSize = 3; break; }
    if (row == nullptr)
        for (int_t id : BOTTOM_BUTTONS)
            if (id == controllerFocus) { row = BOTTOM_BUTTONS; rowSize = 4; break; }
    if (row == nullptr)
        return;

    int_t current = 0;
    while (current < rowSize && row[current] != controllerFocus) ++current;
    for (int_t candidate = current + direction; candidate >= 0 && candidate < rowSize; candidate += direction)
    {
        GuiButton *button = findButton(row[candidate]);
        if (button != nullptr && button->enabled && button->enabled2)
        {
            focusButton(row[candidate]);
            return;
        }
    }
#else
    (void)direction;
#endif
}

void GuiMultiplayer::moveControllerFocusVertical(int_t direction)
{
#if defined(PS2_PLATFORM) || (defined(CTR_PLATFORM) && !defined(NO_NETWORK))
    if (direction == 0)
        return;
    if (controllerFocus == SERVER_LIST_FOCUS)
    {
        const int_t count = static_cast<int_t>(serverList.size());
        const int_t candidate = selectedServer + direction;
        if (candidate >= 0 && candidate < count)
        {
            setSelectedServer(candidate);
            if (mc != nullptr && mc->sndManager != nullptr)
                mc->sndManager->playSoundFX("random.focus", 1.0f, 1.0f);
            return;
        }
        if (direction > 0)
            moveControllerFocusHorizontal(1);
        return;
    }

    bool inTopRow = false;
    bool inBottomRow = false;
    for (int_t id : TOP_BUTTONS) inTopRow = inTopRow || id == controllerFocus;
    for (int_t id : BOTTOM_BUTTONS) inBottomRow = inBottomRow || id == controllerFocus;
    if (direction < 0 && inTopRow)
    {
        if (!serverList.empty())
        {
            controllerFocus = SERVER_LIST_FOCUS;
            syncControllerFocus();
            if (mc != nullptr && mc->sndManager != nullptr)
                mc->sndManager->playSoundFX("random.focus", 1.0f, 1.0f);
        }
        return;
    }
    if ((direction > 0 && inBottomRow) || (direction < 0 && !inBottomRow) ||
        (direction > 0 && !inTopRow))
        return;

    const int_t *targetRow = direction > 0 ? BOTTOM_BUTTONS : TOP_BUTTONS;
    const int_t targetSize = direction > 0 ? 4 : 3;
    GuiButton *current = findButton(controllerFocus);
    if (current == nullptr)
        return;
    const int_t currentX = current->xPosition + current->getButtonWidth() / 2;
    int_t bestId = -1;
    int_t bestDistance = 0x7fffffff;
    for (int_t i = 0; i < targetSize; ++i)
    {
        GuiButton *candidate = findButton(targetRow[i]);
        if (candidate == nullptr || !candidate->enabled || !candidate->enabled2)
            continue;
        int_t distance = candidate->xPosition + candidate->getButtonWidth() / 2 - currentX;
        if (distance < 0) distance = -distance;
        if (distance < bestDistance)
        {
            bestDistance = distance;
            bestId = targetRow[i];
        }
    }
    if (bestId >= 0)
        focusButton(bestId);
#else
    (void)direction;
#endif
}

void GuiMultiplayer::activateControllerFocus()
{
#if defined(PS2_PLATFORM) || (defined(CTR_PLATFORM) && !defined(NO_NETWORK))
    if (mc != nullptr && mc->sndManager != nullptr)
        mc->sndManager->playSoundFX("random.action", 1.0f, 1.0f);
    if (controllerFocus == SERVER_LIST_FOCUS)
    {
        joinServer(selectedServer);
        return;
    }
    GuiButton *button = findButton(controllerFocus);
    if (button != nullptr && button->enabled && button->enabled2)
        actionPerformed(button);
#endif
}

void GuiMultiplayer::handleSpecializedMenuInput()
{
    // The runtime gate replaces the old PS2-only #ifdef: it keeps PC/Wii and
    // NO_NETWORK builds out of the pad focus model exactly when
    // usesSpecializedMenuNavigation() said they were out, and
    // GuiScreen::handleInput() calls this on every platform.
    if (!usesSpecializedMenuNavigation())
        return;

    // platformTextInputSnapshot() consumes the latched pressed bits
    // (consume-on-read), so the specialized owner must be first in the chain
    // -- before VirtualKeyboard::tick() and ContainerSlotNavigator::tick()
    // -- exactly like GuiIngameMenu's comment warns.
    const PlatformTextInputSnapshot pad = platformTextInputSnapshot(platformMenuPad());
#if defined(CTR_PLATFORM)
    // One-shot opening-turn latch, seeded by initGui() on every (re)entry.
    // The A that opened this screen -- or confirmed a sub-screen that came
    // back to it -- leaves its TYPE edge sitting in the pad latch, because
    // the legacy menus it is reached from navigate through the synthesized
    // KEY_* channel and never consume that latch; DsInput only clears it on
    // the gameplay boundary, so menu-to-menu edges survive the switch. The
    // snapshot read above consumed the bits either way; the opening turn
    // just refuses to act on them, mirroring GuiIngameMenu's PS2 pause
    // re-arm latches. Without this, entering the screen auto-joined the
    // first server in the list (2026-09-29, 3DS).
    const bool openingTurn = padOpeningTurnArmed;
    padOpeningTurnArmed = false;
    if (openingTurn)
        return;
    // While the finger owns the panel the touch selection wins: pad navigation
    // would fight it and the finger is already ON the thing being chosen. The
    // snapshot read above already consumed this frame's edges, so they do not
    // replay as a stale navigation step when the finger lifts.
    if (platformMenuPointerActive())
        return;
    // Y = "exit/back-out" in this console's menu table (DsInput.cpp). B and
    // START go back through the KEY_ESCAPE channel keyTyped handles below;
    // the PS2's SHIFT half is not carried over because on this console R is
    // the on-screen keyboard's shift, not a back button.
    constexpr std::uint32_t backMask = PLATFORM_TEXT_CLOSE;
#else
    constexpr std::uint32_t backMask = PLATFORM_TEXT_CLOSE | PLATFORM_TEXT_SHIFT;
#endif
    if ((pad.pressed & backMask) != 0)
    {
        if (mc != nullptr && mc->sndManager != nullptr)
            mc->sndManager->playSoundFX("random.back", 1.0f, 1.0f);
        GuiButton *cancel = findButton(0);
        if (cancel != nullptr) actionPerformed(cancel);
        return;
    }
    if ((pad.pressed & PLATFORM_TEXT_LEFT) != 0) moveControllerFocusHorizontal(-1);
    else if ((pad.pressed & PLATFORM_TEXT_RIGHT) != 0) moveControllerFocusHorizontal(1);
    else if ((pad.pressed & PLATFORM_TEXT_UP) != 0) moveControllerFocusVertical(-1);
    else if ((pad.pressed & PLATFORM_TEXT_DOWN) != 0) moveControllerFocusVertical(1);
    if ((pad.pressed & (PLATFORM_TEXT_TYPE | PLATFORM_TEXT_ENTER)) != 0)
        activateControllerFocus();
}

void GuiMultiplayer::keyTyped(char_t c, int_t key)
{
    // On the pad-driven platforms A arrives TWICE: as the PLATFORM_TEXT_TYPE
    // latch handleSpecializedMenuInput() activates the focused control with,
    // and as the synthesized KEY_RETURN the input backend pushes for the
    // same button (PS2's mapper skips that push for specialized screens; the
    // 3DS's does not). Acting on RETURN here as well would fire the focused
    // button AND the Select button on one press. Only the non-specialized
    // platforms keep the classic "Enter joins the selection" shortcut.
    if ((c == '\r' || key == lwjgl::Keyboard::KEY_RETURN) && buttonSelect != nullptr &&
        !usesSpecializedMenuNavigation())
        actionPerformed(buttonSelect);
    else if (key == lwjgl::Keyboard::KEY_ESCAPE && mc != nullptr)
        mc->displayGuiScreen(parentScreen);
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
#ifdef NO_NETWORK
    drawCenteredString(fontRenderer, translate->translateKey("multiplayer.title"), width / 2, 20, 0xffffff);
    drawCenteredString(fontRenderer, uiText("Online multiplayer is not available on this platform."),
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
void GuiMultiplayer::setSelectedServer(int_t index)
{
    selectedServer = index >= 0 && index < static_cast<int_t>(serverList.size()) ? index : -1;
    updateSelectionButtons();
    if (serverSlotContainer != nullptr && selectedServer >= 0)
        serverSlotContainer->scrollToElement(selectedServer);
}
GuiButton *GuiMultiplayer::getButtonSelect() const { return buttonSelect; }
GuiButton *GuiMultiplayer::getButtonEdit() const { return buttonEdit; }
GuiButton *GuiMultiplayer::getButtonDelete() const { return buttonDelete; }
void GuiMultiplayer::setTooltipText(const std::string &text) { lagTooltip = text; }
int_t GuiMultiplayer::getThreadsPending() { return threadsPending.load(); }
void GuiMultiplayer::incrementThreadsPending() { threadsPending.fetch_add(1); }
void GuiMultiplayer::decrementThreadsPending() { threadsPending.fetch_sub(1); }

void GuiMultiplayer::pollServer(const std::shared_ptr<ServerNBTStorage> &server)
{
#ifdef NO_NETWORK
    if (server != nullptr)
    {
        server->lag = -1;
        server->motd = "\xC2\xA7" "4Can't reach server";
    }
#else
    if (server == nullptr)
        return;

    auto markOffline = [&server]() {
        std::lock_guard<std::mutex> guard(server->stateMutex);
        server->lag = -1;
        server->motd = "\xC2\xA7" "4Can't reach server";
        server->playerCount = "\xC2\xA7" "8???";
    };

    std::string host;
    int_t port = 25565;
    splitServerAddress(server->host, host, port);

    std::unique_ptr<JavaNetwork::Socket> socket = JavaNetwork::createSocket();
    if (socket == nullptr || !socket->connect(host, port))
    {
        markOffline();
        return;
    }

    std::unique_ptr<std::istream> input = JavaNetwork::createInputStream(*socket);
    const char ping = (char)254;
    if (!socket->write(&ping, 1))
    {
        markOffline();
        return;
    }

    const int packetId = input->get();
    if (packetId != 255)
    {
        markOffline();
        return;
    }

    std::string response;
    try
    {
        response = Packet::readString(*input, 256);
    }
    catch (...)
    {
        markOffline();
        return;
    }
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