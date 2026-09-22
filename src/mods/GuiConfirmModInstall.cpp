#include "GuiConfirmModInstall.h"
#include "GuiButton.h"
#include "Minecraft.h"
#include "FontRenderer.h"
#include "SoundManager.h"
#include "ModManager.h"
#include "GuiMods.h"

#if PLATFORM_PS2 || PLATFORM_WII
#include "platform/Input.h"
#endif

GuiConfirmModInstall::GuiConfirmModInstall(GuiScreen *parent, const OchPackInfo &pack)
    : parentScreen(parent)
    , packInfo(pack)
{
    installedVersion = ModManager::getInstance().getInstalledModVersion(pack.id);
}

void GuiConfirmModInstall::initGui()
{
    controlList.clear();

    std::string btnText = installedVersion.empty() ? "Install" : "Update / Reinstall";
    controlList.push_back(new GuiButton(1, width / 2 - 155, height / 4 + 115, 150, 20, btnText));
    controlList.push_back(new GuiButton(2, width / 2 + 5, height / 4 + 115, 150, 20, "Cancel"));
}

void GuiConfirmModInstall::actionPerformed(GuiButton *button)
{
    if (!button->enabled)
        return;

    if (button->id == 1) // Install
    {
        std::string err;
        bool ok = ModManager::getInstance().installModPack(packInfo.filePath, err);
        if (ok)
        {
            if (mc != nullptr && mc->sndManager != nullptr)
                mc->sndManager->playSoundFX("random.click", 1.0f, 1.0f);

            // Navigate back to the mod manager screen to see the newly installed mod
            mc->displayGuiScreen(new GuiMods(nullptr));
        }
        else
        {
            statusMessage = "Error: " + err;
            isError = true;
        }
    }
    else if (button->id == 2) // Cancel
    {
        mc->displayGuiScreen(parentScreen);
    }
}

void GuiConfirmModInstall::keyTyped(char_t c, int_t key)
{
    if (key == 1) // ESC
    {
        mc->displayGuiScreen(parentScreen);
        return;
    }
    GuiScreen::keyTyped(c, key);
}

void GuiConfirmModInstall::handleSpecializedMenuInput()
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

void GuiConfirmModInstall::drawScreen(int_t mouseX, int_t mouseY, float_t partialTick)
{
    drawDefaultBackground();

    // Title
    drawCenteredString(fontRenderer, "Install Mod Package", width / 2, 20, 0xFFFFFF);

    // Box details
    int_t startY = height / 4 + 10;

    std::string nameLine = std::string("\xc2\xa7") + "e" + packInfo.name;
    drawCenteredString(fontRenderer, nameLine, width / 2, startY, 0xFFFFFF);

    std::string authorLine = std::string("\xc2\xa7") + "7" + "By: " + (packInfo.author.empty() ? "Unknown" : packInfo.author);
    drawCenteredString(fontRenderer, authorLine, width / 2, startY + 16, 0x888888);

    std::string descLine = std::string("\xc2\xa7") + "8" + packInfo.description;
    drawCenteredString(fontRenderer, descLine, width / 2, startY + 32, 0x888888);

    std::string pkgLine = std::string("\xc2\xa7") + "f" + "Package Version: " + std::string("\xc2\xa7") + "a" + packInfo.version;
    drawCenteredString(fontRenderer, pkgLine, width / 2, startY + 54, 0xFFFFFF);

    std::string instLine;
    if (installedVersion.empty())
    {
        instLine = std::string("\xc2\xa7") + "f" + "Installed Version: " + std::string("\xc2\xa7") + "c" + "None (Not Installed)";
    }
    else
    {
        instLine = std::string("\xc2\xa7") + "f" + "Installed Version: " + std::string("\xc2\xa7") + "e" + installedVersion;
    }
    drawCenteredString(fontRenderer, instLine, width / 2, startY + 70, 0xFFFFFF);

    if (!statusMessage.empty())
    {
        int_t color = isError ? 0xFF5555 : 0x55FF55;
        drawCenteredString(fontRenderer, statusMessage, width / 2, startY + 92, color);
    }

    GuiScreen::drawScreen(mouseX, mouseY, partialTick);
}
