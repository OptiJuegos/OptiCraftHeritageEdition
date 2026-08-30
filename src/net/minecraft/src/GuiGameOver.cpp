#include "GuiGameOver.h"
#include "GuiButton.h"
#include "GuiMainMenu.h"
#include "FontRenderer.h"
#include "Minecraft.h"
#include "EntityPlayerSP.h"
#include "World.h"
#include "WorldInfo.h"
#include "ISaveHandler.h"
#include "ISaveFormat.h"
#include "StatCollector.h"
#include "platform/RenderAPI.h"
#include <string>

GuiGameOver::GuiGameOver() :
    cooldownTimer(0)
{
}

void GuiGameOver::initGui()
{
    controlList.clear();
    cooldownTimer = 0;

    const bool hardcore = mc->theWorld != nullptr && mc->theWorld->getWorldInfo() != nullptr &&
                          mc->theWorld->getWorldInfo()->isHardcoreModeEnabled();
    if (hardcore)
    {
        controlList.push_back(new GuiButton(1, width / 2 - 100, height / 4 + 96,
                                            StatCollector::translateToLocal("deathScreen.deleteWorld")));
    }
    else
    {
        controlList.push_back(new GuiButton(1, width / 2 - 100, height / 4 + 72,
                                            StatCollector::translateToLocal("deathScreen.respawn")));
        controlList.push_back(new GuiButton(2, width / 2 - 100, height / 4 + 96,
                                            StatCollector::translateToLocal("deathScreen.titleScreen")));
        if (mc->session == nullptr && controlList.size() > 1)
            controlList[1]->enabled = false;
    }

    for (GuiButton *button : controlList)
        button->enabled = false;
}

void GuiGameOver::keyTyped(char_t c, int_t i)
{
}

void GuiGameOver::actionPerformed(GuiButton *guibutton)
{
    if (guibutton == nullptr || !guibutton->enabled)
        return;

    switch (guibutton->id)
    {
        case 1:
        {
            const bool hardcore = mc->theWorld != nullptr && mc->theWorld->getWorldInfo() != nullptr &&
                                  mc->theWorld->getWorldInfo()->isHardcoreModeEnabled();
            if (hardcore)
            {
                ISaveHandler *saveHandler = mc->theWorld->getSaveHandler();
                const std::string saveDirectoryName = saveHandler != nullptr ? saveHandler->getSaveDirectoryName() : std::string();
                mc->changeWorld2(nullptr, "Deleting world");

                ISaveFormat *saveFormat = mc->getSaveLoader();
                if (saveFormat != nullptr && !saveDirectoryName.empty())
                {
                    saveFormat->flushCache();
                    saveFormat->deleteWorldDirectory(saveDirectoryName);
                }
                mc->displayGuiScreen(new GuiMainMenu());
            }
            else if (mc->thePlayer != nullptr)
            {
                mc->thePlayer->respawnPlayer();
                mc->displayGuiScreen(nullptr);
            }
            break;
        }

        case 2:
            if (mc->isMultiplayerWorld() && mc->theWorld != nullptr)
                mc->theWorld->sendQuittingDisconnectingPacket();
            mc->changeWorld1(nullptr);
            mc->displayGuiScreen(new GuiMainMenu());
            break;
    }
}

void GuiGameOver::drawScreen(int_t i, int_t j, float_t f)
{
    drawGradientRect(0, 0, width, height, 0x60500000, 0xa0803030);
    renderPushMatrix();
    renderScale(2.0f, 2.0f, 2.0f);

    const bool hardcore = mc->theWorld != nullptr && mc->theWorld->getWorldInfo() != nullptr &&
                          mc->theWorld->getWorldInfo()->isHardcoreModeEnabled();
    const std::string title = StatCollector::translateToLocal(hardcore ? "deathScreen.title.hardcore" : "deathScreen.title");
    drawCenteredString(fontRenderer, title, width / 2 / 2, 30, 0xffffff);
    renderPopMatrix();

    if (hardcore)
        drawCenteredString(fontRenderer, StatCollector::translateToLocal("deathScreen.hardcoreInfo"), width / 2, 144, 0xffffff);

    drawCenteredString(fontRenderer,
                       StatCollector::translateToLocal("deathScreen.score") + ": §e" + std::to_string(mc->thePlayer->getScore()),
                       width / 2, 100, 0xffffff);
    GuiScreen::drawScreen(i, j, f);
}

bool GuiGameOver::doesGuiPauseGame()
{
    return false;
}

void GuiGameOver::updateScreen()
{
    GuiScreen::updateScreen();
    ++cooldownTimer;
    if (cooldownTimer == 20)
    {
        for (GuiButton *button : controlList)
            button->enabled = true;
    }
}
