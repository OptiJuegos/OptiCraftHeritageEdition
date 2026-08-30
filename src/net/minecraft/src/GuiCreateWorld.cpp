#include "GuiCreateWorld.h"
#include "GuiTextField.h"
#include "GuiButton.h"
#include "StringTranslate.h"
#include "StatCollector.h"
#include "ChatAllowedCharacters.h"
#include "MathHelper.h"
#include "ISaveFormat.h"
#include "WorldInfo.h"
#include "WorldSettings.h"
#include "WorldType.h"
#include "PlayerControllerSP.h"
#include "PlayerControllerCreative.h"
#include "Minecraft.h"
#include "java/Random.h"
#include "java/String.h"
#include "pc/lwjgl/Keyboard.h"
#include <algorithm>

GuiCreateWorld::GuiCreateWorld(GuiScreen *parent)
    : parentScreen(parent)
    , textboxWorldName(nullptr)
    , textboxSeed(nullptr)
    , gameMode("survival")
    , generateStructures(true)
    , hardcore(false)
    , createClicked(false)
    , moreOptions(false)
    , gameModeButton(nullptr)
    , moreWorldOptionsButton(nullptr)
    , generateStructuresButton(nullptr)
    , worldTypeButton(nullptr)
    , seed("")
    , localizedNewWorldText(StatCollector::translateToLocal("selectWorld.newWorld"))
    , worldTypeIndex(0)
{
}

GuiCreateWorld::~GuiCreateWorld()
{
    delete textboxWorldName;
    delete textboxSeed;
}

void GuiCreateWorld::updateScreen()
{
    textboxWorldName->updateCursorCounter();
    textboxSeed->updateCursorCounter();
}

void GuiCreateWorld::initGui()
{
    StringTranslate *tr = StringTranslate::getInstance();
    WorldType::initialize();
    lwjgl::Keyboard::enableRepeatEvents(true);
    controlList.clear();

    controlList.push_back(new GuiButton(0, width / 2 - 155, height - 28, 150, 20,
        tr->translateKey("selectWorld.create")));
    controlList.push_back(new GuiButton(1, width / 2 + 5, height - 28, 150, 20,
        tr->translateKey("gui.cancel")));
    controlList.push_back(gameModeButton = new GuiButton(2, width / 2 - 75, 100, 150, 20,
        tr->translateKey("selectWorld.gameMode")));
    controlList.push_back(moreWorldOptionsButton = new GuiButton(3, width / 2 - 75, 172, 150, 20,
        tr->translateKey("selectWorld.moreWorldOptions")));
    controlList.push_back(generateStructuresButton = new GuiButton(4, width / 2 - 155, 100, 150, 20,
        tr->translateKey("selectWorld.mapFeatures")));
    generateStructuresButton->enabled2 = false;
    controlList.push_back(worldTypeButton = new GuiButton(5, width / 2 + 5, 100, 150, 20,
        tr->translateKey("selectWorld.mapType")));
    worldTypeButton->enabled2 = false;

    delete textboxWorldName;
    textboxWorldName = new GuiTextField(fontRenderer, width / 2 - 100, 60, 200, 20);
    textboxWorldName->setFocused(true);
    textboxWorldName->setText(localizedNewWorldText);

    delete textboxSeed;
    textboxSeed = new GuiTextField(fontRenderer, width / 2 - 100, 60, 200, 20);
    textboxSeed->setText(seed);

    updateFolderName();
    updateButtonText();
}

void GuiCreateWorld::updateFolderName()
{
    folderName = String::trimJava(textboxWorldName->getText());
    for (char c : ChatAllowedCharacters::allowedCharactersArray)
        std::replace(folderName.begin(), folderName.end(), c, '_');
    if (MathHelper::stringNullOrLengthZero(folderName))
        folderName = "World";
    folderName = generateUnusedFolderName(mc->getSaveLoader(), folderName);
}

void GuiCreateWorld::updateButtonText()
{
    StringTranslate *tr = StringTranslate::getInstance();
    gameModeButton->displayString = tr->translateKey("selectWorld.gameMode") + " " +
        tr->translateKey("selectWorld.gameMode." + gameMode);
    gameModeDescriptionLine1 = tr->translateKey("selectWorld.gameMode." + gameMode + ".line1");
    gameModeDescriptionLine2 = tr->translateKey("selectWorld.gameMode." + gameMode + ".line2");

    generateStructuresButton->displayString = tr->translateKey("selectWorld.mapFeatures") + " " +
        tr->translateKey(generateStructures ? "options.on" : "options.off");

    WorldType *type = nullptr;
    if (worldTypeIndex >= 0 && worldTypeIndex < WorldType::WORLD_TYPE_COUNT)
        type = WorldType::worldTypes[worldTypeIndex];
    if (type == nullptr)
        type = WorldType::DEFAULT;
    worldTypeButton->displayString = tr->translateKey("selectWorld.mapType") + " " +
        tr->translateKey(type->getTranslateName());
}

std::string GuiCreateWorld::generateUnusedFolderName(ISaveFormat *fmt, const std::string &base)
{
    std::string result = base;
    for (char c : std::string("./\""))
        std::replace(result.begin(), result.end(), c, '_');

    std::size_t pos = 0;
    while ((pos = result.find("COM", pos)) != std::string::npos)
    {
        result.replace(pos, 3, "_");
        ++pos;
    }

    while (true)
    {
        WorldInfo *info = fmt->getWorldInfo(result);
        if (info == nullptr)
            break;
        delete info;
        result += "-";
    }
    return result;
}

void GuiCreateWorld::onGuiClosed()
{
    lwjgl::Keyboard::enableRepeatEvents(false);
}

void GuiCreateWorld::actionPerformed(GuiButton *button)
{
    if (!button->enabled)
        return;

    if (button->id == 1)
    {
        mc->displayGuiScreen(parentScreen);
    }
    else if (button->id == 0)
    {
        mc->displayGuiScreen(nullptr);
        if (createClicked)
            return;
        createClicked = true;

        Random random;
        long_t worldSeed = random.nextLong();
        const std::string seedText = textboxSeed->getText();
        if (!MathHelper::stringNullOrLengthZero(seedText))
        {
            long_t parsed = 0;
            if (String::tryParseLong(seedText, parsed))
            {
                if (parsed != 0LL)
                    worldSeed = parsed;
            }
            else
            {
                worldSeed = static_cast<long_t>(String::hashCode(seedText));
            }
        }

        int_t gameType = 0;
        delete mc->playerController;
        if (gameMode == "creative")
        {
            gameType = 1;
            mc->playerController = new PlayerControllerCreative(mc);
        }
        else
        {
            mc->playerController = new PlayerControllerSP(mc);
        }

        WorldType *type = WorldType::DEFAULT;
        if (worldTypeIndex >= 0 && worldTypeIndex < WorldType::WORLD_TYPE_COUNT &&
            WorldType::worldTypes[worldTypeIndex] != nullptr)
        {
            type = WorldType::worldTypes[worldTypeIndex];
        }

        WorldSettings settings(worldSeed, gameType, generateStructures, hardcore, type);
        mc->startWorld(folderName, textboxWorldName->getText(), &settings);
        mc->displayGuiScreen(nullptr);
    }
    else if (button->id == 3)
    {
        moreOptions = !moreOptions;
        gameModeButton->enabled2 = !moreOptions;
        generateStructuresButton->enabled2 = moreOptions;
        worldTypeButton->enabled2 = moreOptions;
        moreWorldOptionsButton->displayString = StringTranslate::getInstance()->translateKey(
            moreOptions ? "gui.done" : "selectWorld.moreWorldOptions");
    }
    else if (button->id == 2)
    {
        if (gameMode == "survival")
        {
            gameMode = "hardcore";
            hardcore = true;
        }
        else if (gameMode == "hardcore")
        {
            gameMode = "creative";
            hardcore = false;
        }
        else
        {
            gameMode = "survival";
            hardcore = false;
        }
        updateButtonText();
    }
    else if (button->id == 4)
    {
        generateStructures = !generateStructures;
        updateButtonText();
    }
    else if (button->id == 5)
    {
        do
        {
            ++worldTypeIndex;
            if (worldTypeIndex >= WorldType::WORLD_TYPE_COUNT)
                worldTypeIndex = 0;
        }
        while (WorldType::worldTypes[worldTypeIndex] == nullptr ||
               !WorldType::worldTypes[worldTypeIndex]->getCanBeCreated());
        updateButtonText();
    }
}

void GuiCreateWorld::keyTyped(char_t c, int_t key)
{
    if (textboxWorldName->getFocused() && !moreOptions)
    {
        textboxWorldName->textboxKeyTyped(c, key);
        localizedNewWorldText = textboxWorldName->getText();
    }
    else if (textboxSeed->getFocused() && moreOptions)
    {
        textboxSeed->textboxKeyTyped(c, key);
        seed = textboxSeed->getText();
    }

    if (c == '\r')
        actionPerformed(controlList[0]);

    controlList[0]->enabled = !textboxWorldName->getText().empty();
    updateFolderName();
}

void GuiCreateWorld::mouseClicked(int_t x, int_t y, int_t button)
{
    GuiScreen::mouseClicked(x, y, button);
    if (!moreOptions)
        textboxWorldName->mouseClicked(x, y, button);
    else
        textboxSeed->mouseClicked(x, y, button);
}

void GuiCreateWorld::drawScreen(int_t mouseX, int_t mouseY, float_t partialTick)
{
    StringTranslate *tr = StringTranslate::getInstance();
    drawDefaultBackground();
    drawCenteredString(fontRenderer, tr->translateKey("selectWorld.create"), width / 2, 20, 0xffffff);

    if (!moreOptions)
    {
        drawString(fontRenderer, tr->translateKey("selectWorld.enterName"), width / 2 - 100, 47, 0xa0a0a0);
        drawString(fontRenderer, tr->translateKey("selectWorld.resultFolder") + " " + folderName,
            width / 2 - 100, 85, 0xa0a0a0);
        textboxWorldName->drawTextBox();
        drawString(fontRenderer, gameModeDescriptionLine1, width / 2 - 100, 122, 0xa0a0a0);
        drawString(fontRenderer, gameModeDescriptionLine2, width / 2 - 100, 134, 0xa0a0a0);
    }
    else
    {
        drawString(fontRenderer, tr->translateKey("selectWorld.enterSeed"), width / 2 - 100, 47, 0xa0a0a0);
        drawString(fontRenderer, tr->translateKey("selectWorld.seedInfo"), width / 2 - 100, 85, 0xa0a0a0);
        drawString(fontRenderer, tr->translateKey("selectWorld.mapFeatures.info"), width / 2 - 150, 122, 0xa0a0a0);
        textboxSeed->drawTextBox();
    }

    GuiScreen::drawScreen(mouseX, mouseY, partialTick);
}

void GuiCreateWorld::selectNextField()
{
    if (moreOptions)
    {
        textboxSeed->setFocused(true);
        textboxWorldName->setFocused(false);
    }
    else
    {
        textboxWorldName->setFocused(true);
        textboxSeed->setFocused(false);
    }
}
