#include "GuiLanguage.h"

#include "GameSettings.h"
#include "GuiButton.h"
#include "GuiSlotLanguage.h"
#include "GuiSmallButton.h"
#include "Minecraft.h"
#include "StringTranslate.h"
#include "TexturePackList.h"

GuiLanguage::GuiLanguage(GuiScreen *parent, GameSettings *settingsIn)
    : parentGui(parent)
    , updateTimer(-1)
    , languageList(nullptr)
    , settings(settingsIn)
    , doneButton(nullptr)
{
}

GuiLanguage::~GuiLanguage()
{
    delete languageList;
    languageList = nullptr;
    doneButton = nullptr;
}

void GuiLanguage::initGui()
{
    StringTranslate *translate = StringTranslate::getInstance();
    // No-op off PS2. On PS2, restricts the list this screen shows to
    // languages whose .lang file stays inside Latin-1 (codepoints 0..255) --
    // see the comment on the method for why that is the real boundary, not
    // "uses Latin letters". Lazy/memoized, so this only costs a one-time
    // scan of the language files, paid the first time this screen opens.
    translate->filterToLatinLanguagesOnPs2();
    doneButton = new GuiSmallButton(6, width / 2 - 75, height - 38, translate->translateKey("gui.done"));
    controlList.push_back(doneButton);

    delete languageList;
    languageList = new GuiSlotLanguage(this);
    languageList->registerScrollButtons(controlList, 7, 8);
}

void GuiLanguage::actionPerformed(GuiButton *button)
{
    if (!button->enabled)
        return;

    if (button->id == 6)
    {
        settings->saveOptions();
        mc->displayGuiScreen(parentGui);
    }
    else if (languageList != nullptr)
    {
        languageList->actionPerformed(button);
    }
}

void GuiLanguage::mouseClicked(int_t x, int_t y, int_t button)
{
    GuiScreen::mouseClicked(x, y, button);
}

void GuiLanguage::mouseMovedOrUp(int_t x, int_t y, int_t button)
{
    GuiScreen::mouseMovedOrUp(x, y, button);
}

void GuiLanguage::drawScreen(int_t mouseX, int_t mouseY, float_t partialTick)
{
    if (languageList != nullptr)
        languageList->drawScreen(mouseX, mouseY, partialTick);

    if (updateTimer <= 0 && mc->texturePackList != nullptr)
    {
        mc->texturePackList->updateAvailableTexturePacks();
        updateTimer += 20;
    }

    StringTranslate *translate = StringTranslate::getInstance();
    drawCenteredString(fontRenderer, translate->translateKey("options.language"), width / 2, 16, 0xffffff);
    drawCenteredString(fontRenderer, "(" + translate->translateKey("options.languageWarning") + ")",
                       width / 2, height - 56, 0x808080);
    GuiScreen::drawScreen(mouseX, mouseY, partialTick);
}

void GuiLanguage::updateScreen()
{
    GuiScreen::updateScreen();
    --updateTimer;
}
