#include "net/minecraft/src/UiStrings.h"
#include "LegacyViewOptions.h"

#include "LegacyGuiButton.h"
#include "LegacyOptionCheckbox.h"
#include "LegacyOptionSlider.h"
#include "net/minecraft/src/EnumOptions.h"
#include "net/minecraft/src/GameSettings.h"
#include "net/minecraft/src/GuiButton.h"
#include "net/minecraft/src/Minecraft.h"
#include "net/minecraft/src/StringTranslate.h"
#include "net/minecraft/src/World.h"
#include "net/minecraft/src/WorldInfo.h"
#include "LegacyOptionsLayout.h"

namespace
{
enum LegacyViewButtonId
{
    BUTTON_FOV = 400,
    BUTTON_SENSITIVITY = 401,
    BUTTON_INVERT_MOUSE = 402,
    BUTTON_DIFFICULTY = 403,
    BUTTON_MUSIC = 404,
    BUTTON_SOUND = 405,
    BUTTON_DONE = 499
};

}

LegacyViewOptions::LegacyViewOptions(GuiScreen *parent, GameSettings *settingsValue,
    LegacyOptionsBackgroundMode backgroundModeValue)
    : LegacyOptionsScreen(parent, settingsValue, backgroundModeValue), invertMouseCheckbox(nullptr)
{
}

void LegacyViewOptions::initGui()
{
    configureLegacyLayout(7, true, LegacyOptionsLayoutPreset::Compact);
    const int_t x = legacyLayout.contentX;
    const int_t w = legacyLayout.contentWidth;
    const int_t h = legacyLayout.rowHeight;

    controlList.push_back(new LegacyOptionSlider(BUTTON_FOV, x, legacyLayout.rowY(0), w, h,
        settings, EnumOptions::FOV));
    controlList.push_back(new LegacyOptionSlider(BUTTON_SENSITIVITY, x, legacyLayout.rowY(1), w, h,
        settings, EnumOptions::SENSITIVITY));

    invertMouseCheckbox = new LegacyOptionCheckbox(BUTTON_INVERT_MOUSE, x, legacyLayout.rowY(2), w, h,
        uiText("Invert Mouse"), settings->invertMouse);
    controlList.push_back(invertMouseCheckbox);

    LegacyGuiButton *difficulty = new LegacyGuiButton(BUTTON_DIFFICULTY, x, legacyLayout.rowY(3), w, h,
        settings->getKeyBinding(EnumOptions::DIFFICULTY));
    if (mc != nullptr && mc->theWorld != nullptr && mc->theWorld->getWorldInfo() != nullptr &&
        mc->theWorld->getWorldInfo()->isHardcoreModeEnabled())
    {
        difficulty->enabled = false;
        StringTranslate *tr = StringTranslate::getInstance();
        difficulty->displayString = tr->translateKey("options.difficulty") + ": " +
            tr->translateKey("options.difficulty.hardcore");
    }
    controlList.push_back(difficulty);

    controlList.push_back(new LegacyOptionSlider(BUTTON_MUSIC, x, legacyLayout.rowY(4), w, h,
        settings, EnumOptions::MUSIC));
    controlList.push_back(new LegacyOptionSlider(BUTTON_SOUND, x, legacyLayout.rowY(5), w, h,
        settings, EnumOptions::SOUND));

    controlList.push_back(new LegacyGuiButton(BUTTON_DONE, x, legacyLayout.rowY(6), w, h, uiText("Done")));
}

void LegacyViewOptions::actionPerformed(GuiButton *button)
{
    if (button == nullptr || !button->enabled)
        return;

    switch (button->id)
    {
    case BUTTON_INVERT_MOUSE:
        settings->setOptionValue(EnumOptions::INVERT_MOUSE, 1);
        invertMouseCheckbox->setChecked(settings->invertMouse);
        return;
    case BUTTON_DIFFICULTY:
        settings->setOptionValue(EnumOptions::DIFFICULTY, 1);
        button->displayString = settings->getKeyBinding(EnumOptions::DIFFICULTY);
        return;
    case BUTTON_DONE:
        returnToParent();
        return;
    default:
        return;
    }
}

void LegacyViewOptions::drawScreen(int_t mouseX, int_t mouseY, float_t partialTick)
{
    drawLegacyBackground(partialTick);
    updateLegacyPointerHover(mouseX, mouseY);
    GuiScreen::drawScreen(mouseX, mouseY, partialTick);
}
