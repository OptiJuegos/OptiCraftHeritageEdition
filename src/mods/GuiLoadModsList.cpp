#include "GuiLoadModsList.h"
#include "GuiSlot.h"
#include "GuiButton.h"
#include "Minecraft.h"
#include "FontRenderer.h"
#include "SoundManager.h"
#include "ModManager.h"
#include "GuiConfirmModInstall.h"
#include "platform/Storage.h"

#ifdef PS2_PLATFORM
#include "ps2/storage/assets/Ps2Assets.h"
#endif

#if PLATFORM_PS2 || PLATFORM_WII
#include "platform/Input.h"
#endif

class GuiSlotLoadMods : public GuiSlot
{
public:
    GuiSlotLoadMods(GuiLoadModsList *parentScreen)
        : GuiSlot(parentScreen->mc, parentScreen->width, parentScreen->height, 36, parentScreen->height - 38, 36)
        , parent(parentScreen)
    {
        setShowSelectionBox(true);
    }

    int_t getSize() override
    {
        return static_cast<int_t>(parent->getPacks().size());
    }

    void elementClicked(int_t index, bool doubleClicked) override
    {
        const auto &packs = parent->getPacks();
        if (index >= 0 && index < static_cast<int_t>(packs.size()))
        {
            parent->setSelectedPackIndex(index);

            if (parent->mc != nullptr && parent->mc->sndManager != nullptr)
                parent->mc->sndManager->playSoundFX("random.click", 1.0f, 1.0f);

            if (doubleClicked)
            {
                parent->setPendingConfirmIndex(index);
            }
        }
    }

    bool isSelected(int_t index) override
    {
        return index == parent->getSelectedPackIndex();
    }

    int_t getContentHeight() override
    {
        return getSize() * 36;
    }

    void drawBackground() override
    {
        parent->drawDefaultBackground();
    }

    void drawSlot(int_t index, int_t x, int_t y, int_t height, Tessellator *tess) override
    {
        const auto &packs = parent->getPacks();
        if (index < 0 || index >= static_cast<int_t>(packs.size()))
            return;

        const auto &pack = packs[index];
        FontRenderer *fr = parent->getFont();
        if (fr == nullptr)
            return;

        // Title & Version
        std::string title = pack.name + " " + std::string("\xc2\xa7") + "7" + pack.version;
        parent->drawString(fr, title, x + 4, y + 4, 0xFFFFFF);

        // Description & Author
        std::string desc = pack.description;
        if (!pack.author.empty())
            desc += " " + std::string("\xc2\xa7") + "8(" + pack.author + ")";
        parent->drawString(fr, desc, x + 4, y + 18, 0x888888);

        // Badge on right
        int_t btnW = 76;
        int_t btnH = 18;
        int_t btnX = x + 216 - btnW;
        int_t btnY = y + 6;

        std::string installedVer = ModManager::getInstance().getInstalledModVersion(pack.id);
        bool isInstalled = !installedVer.empty();
        bool isUpdate = isInstalled && (installedVer != pack.version);

        int_t bgColor = isInstalled ? (isUpdate ? 0x80503010 : 0x80104510) : 0x80202030;
        int_t borderColor = isInstalled ? (isUpdate ? 0xFFA06020 : 0xFF28A828) : 0xFF405070;

        parent->drawRect(btnX, btnY, btnX + btnW, btnY + btnH, bgColor);
        parent->drawRect(btnX, btnY, btnX + btnW, btnY + 1, borderColor);
        parent->drawRect(btnX, btnY + btnH - 1, btnX + btnW, btnY + btnH, borderColor);
        parent->drawRect(btnX, btnY, btnX + 1, btnY + btnH, borderColor);
        parent->drawRect(btnX + btnW - 1, btnY, btnX + btnW, btnY + btnH, borderColor);

        std::string statusText;
        if (isUpdate)
            statusText = std::string("\xc2\xa7") + "6[ UPDATE ]";
        else if (isInstalled)
            statusText = std::string("\xc2\xa7") + "a[ INSTALLED ]";
        else
            statusText = std::string("\xc2\xa7") + "b[ AVAILABLE ]";

        parent->drawCenteredString(fr, statusText, btnX + btnW / 2, btnY + 5, 0xFFFFFF);
    }

private:
    GuiLoadModsList *parent;
};

GuiLoadModsList::GuiLoadModsList(GuiScreen *parent, Source source)
    : parentScreen(parent)
    , loadSource(source)
    , slotList(nullptr)
{
    if (loadSource == Source::Device)
    {
        screenTitle = "Available Mods (Device)";
        emptyMessage1 = "No .ochpack packages found on device.";
        emptyMessage2 = "Place .ochpack files in the 'mods' folder next to the ELF.";
    }
    else
    {
        screenTitle = "Available Mods (USB Storage)";
        emptyMessage1 = "No .ochpack packages found on USB storage.";
        emptyMessage2 = "Checked: mass:/ and mass:/mods/. Ensure USB is connected.";
    }
}

GuiLoadModsList::~GuiLoadModsList()
{
    delete slotList;
    slotList = nullptr;
}

void GuiLoadModsList::scanPacks()
{
    availablePacks.clear();
    selectedIndex = -1;
    debugLogs.clear();

    std::vector<std::string> scanDirs;

    if (loadSource == Source::Device)
    {
#ifdef PS2_PLATFORM
        std::string inst = Ps2Assets::installDir();
        debugLogs.push_back("installDir: '" + inst + "'");
        if (!inst.empty())
        {
            scanDirs.push_back(PlatformStorage::join(inst, "mods"));
            scanDirs.push_back(PlatformStorage::join(inst, "MODS"));
            if (inst.back() == ':')
            {
                scanDirs.push_back(inst + "mods");
                scanDirs.push_back(inst + "/mods");
                scanDirs.push_back(inst + "/MODS");
            }
        }
        scanDirs.push_back("cdrom0:/mods");
        scanDirs.push_back("cdrom0:/MODS");
        scanDirs.push_back("host:mods");
        scanDirs.push_back("host:/mods");
#endif
        scanDirs.push_back("./mods");
        scanDirs.push_back("mods");
    }
    else
    {
        scanDirs.push_back("mass:/OptiCraftHeritage/mods");
        scanDirs.push_back("mass:/mods");
        scanDirs.push_back("mass:/MODS");
        scanDirs.push_back("mass0:/OptiCraftHeritage/mods");
        scanDirs.push_back("mass0:/mods");
        scanDirs.push_back("usb/mods");
    }

    for (const auto &dir : scanDirs)
    {
        auto packs = OchPackReader::scanDirectory(dir, &debugLogs);
        for (const auto &p : packs)
        {
            bool duplicate = false;
            for (const auto &existing : availablePacks)
            {
                if (existing.id == p.id)
                {
                    duplicate = true;
                    break;
                }
            }
            if (!duplicate)
            {
                availablePacks.push_back(p);
            }
        }

        // If we found packs in this primary directory, stop probing fallback directories
        if (!availablePacks.empty())
        {
            break;
        }
    }
}

void GuiLoadModsList::setSelectedPackIndex(int_t index)
{
    selectedIndex = index;
    for (auto *btn : controlList)
    {
        if (btn->id == 1) // Install button
        {
            btn->enabled = (selectedIndex >= 0 && selectedIndex < static_cast<int_t>(availablePacks.size()));
        }
    }
}

void GuiLoadModsList::initGui()
{
    if (!scanned_)
    {
        scanPacks();
        scanned_ = true;
    }

    delete slotList;
    slotList = nullptr;

    controlList.clear();

    if (!availablePacks.empty())
    {
        slotList = new GuiSlotLoadMods(this);
        slotList->registerScrollButtons(controlList, 7, 8);

        controlList.push_back(new GuiButton(1, width / 2 - 155, height - 32, 150, 20, "Install Selected"));
        controlList.push_back(new GuiButton(2, width / 2 + 5, height - 32, 150, 20, "Back"));
        setSelectedPackIndex(selectedIndex);
    }
    else
    {
        controlList.push_back(new GuiButton(2, width / 2 - 75, height - 36, 150, 20, "Back"));
    }
}

void GuiLoadModsList::actionPerformed(GuiButton *button)
{
    if (!button->enabled)
        return;

    if (button->id == 1) // Install Selected
    {
        if (selectedIndex >= 0 && selectedIndex < static_cast<int_t>(availablePacks.size()))
        {
            mc->displayGuiScreen(new GuiConfirmModInstall(this, availablePacks[selectedIndex]));
        }
    }
    else if (button->id == 2) // Back
    {
        mc->displayGuiScreen(parentScreen);
    }
    else if (slotList != nullptr)
    {
        slotList->actionPerformed(button);
    }
}

void GuiLoadModsList::keyTyped(char_t c, int_t key)
{
    if (key == 1) // ESC
    {
        mc->displayGuiScreen(parentScreen);
        return;
    }
    GuiScreen::keyTyped(c, key);
}

void GuiLoadModsList::updateScreen()
{
    GuiScreen::updateScreen();
    if (pendingConfirmIndex >= 0 && pendingConfirmIndex < static_cast<int_t>(availablePacks.size()))
    {
        int_t idx = pendingConfirmIndex;
        pendingConfirmIndex = -1;
        mc->displayGuiScreen(new GuiConfirmModInstall(this, availablePacks[idx]));
    }
}

void GuiLoadModsList::handleSpecializedMenuInput()
{
#if PLATFORM_PS2 || PLATFORM_WII
    const PlatformTextInputSnapshot pad = platformTextInputSnapshot(platformMenuPad());
    if ((pad.pressed & (PLATFORM_TEXT_BACK | PLATFORM_TEXT_CLOSE)) != 0)
    {
        mc->displayGuiScreen(parentScreen);
        return;
    }
#endif
}

void GuiLoadModsList::drawScreen(int_t mouseX, int_t mouseY, float_t partialTick)
{
    if (slotList != nullptr)
    {
        slotList->drawScreen(mouseX, mouseY, partialTick);
    }
    else
    {
        drawDefaultBackground();
        drawCenteredString(fontRenderer, emptyMessage1, width / 2, 45, 0xAAAAAA);
        drawCenteredString(fontRenderer, emptyMessage2, width / 2, 58, 0x777777);

        int_t logY = 74;
        drawString(fontRenderer, std::string("\xc2\xa7") + "6[Diagnostics - Search Log]:", 15, logY, 0xFFAA00);
        logY += 12;
        int_t maxLines = 10;
        for (size_t i = 0; i < debugLogs.size() && (int_t)i < maxLines; ++i)
        {
            drawString(fontRenderer, std::string("\xc2\xa7") + "7" + debugLogs[i], 15, logY, 0x888888);
            logY += 10;
        }
    }

    drawCenteredString(fontRenderer, screenTitle, width / 2, 10, 0xFFFFFF);
    drawCenteredString(fontRenderer, std::string("\xc2\xa7") + "7Select an .ochpack mod to view details and install", width / 2, 23, 0x888888);

    GuiScreen::drawScreen(mouseX, mouseY, partialTick);
}
