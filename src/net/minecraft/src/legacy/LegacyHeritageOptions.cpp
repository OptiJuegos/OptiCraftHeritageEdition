#include "LegacyHeritageOptions.h"

#include "LegacyGuiButton.h"
#include "LegacyHeritagePolicy.h"
#include "LegacyOptionCheckbox.h"
#include "LegacyOptionText.h"
#include "LegacyOptionMetrics.h"
#include "LegacyOptionStyle.h"
#include "net/minecraft/src/EnumOptions.h"
#include "net/minecraft/src/EntityRenderer.h"
#include "net/minecraft/src/GameSettings.h"
#include "net/minecraft/src/GuiButton.h"
#include "net/minecraft/src/GuiDeadzoneSettings.h"
#include "net/minecraft/src/GuiTextField.h"
#include "net/minecraft/src/GuiTextFieldSelector.h"
#include "net/minecraft/src/Minecraft.h"
#include "net/minecraft/src/ScaledResolution.h"
#include "net/minecraft/src/Session.h"
#include "pc/lwjgl/Keyboard.h"
#include "platform/PlatformConfig.h"
#include "platform/PlatformUserSettings.h"

namespace
{
constexpr int_t BUTTON_ASPECT_RATIO = 603;
constexpr int_t BUTTON_LEGACY_UI = 604;
constexpr int_t BUTTON_LEGACY_LOOK = 605;
constexpr int_t BUTTON_ALTERNATIVE_CONTROLS = 601;
constexpr int_t BUTTON_DEADZONE = 602;
constexpr int_t BUTTON_DONE = 600;
constexpr int_t BUTTON_EDIT_PLAYER_NAME = 606;

}

LegacyHeritageOptions::LegacyHeritageOptions(GuiScreen *parent, GameSettings *settingsValue,
    LegacyOptionsBackgroundMode backgroundModeValue)
    : LegacyOptionsScreen(parent, settingsValue, backgroundModeValue), nameField(nullptr), legacyUiCheckbox(nullptr),
      legacyLookCheckbox(nullptr), alternativeControlsCheckbox(nullptr)
{
}

LegacyHeritageOptions::~LegacyHeritageOptions()
{
    delete nameField;
    nameField = nullptr;
}

void LegacyHeritageOptions::initGui()
{
    int_t rowCount = 5; // player name label, player name field, Legacy UI, Legacy Look, Done
#if PLATFORM_HAS_ASPECT_RATIO_OPTION
    ++rowCount;
#endif
#ifdef WII_PLATFORM
    ++rowCount;
#endif
#if PLATFORM_HAS_CONTROLLER_CALIBRATION
    ++rowCount;
#endif

    configureLegacyLayout(rowCount, true, LegacyOptionsLayoutPreset::Compact);
    const int_t x = legacyLayout.contentX;
    const int_t w = legacyLayout.contentWidth;
    const int_t h = legacyLayout.rowHeight;
    int_t row = 0;

    delete nameField;
    const int_t nameFieldInset = 2;
    nameField = new GuiTextField(this, fontRenderer, x + nameFieldInset, legacyLayout.rowY(row + 1),
        std::max<int_t>(1, w - nameFieldInset * 2), h,
        settings != nullptr ? settings->playerName : "Player");
    nameField->setMaxStringLength(16);
    nameField->setFocused(false);
    controlList.push_back(new GuiTextFieldSelector(BUTTON_EDIT_PLAYER_NAME,
        x + nameFieldInset, legacyLayout.rowY(row + 1),
        std::max<int_t>(1, w - nameFieldInset * 2), h));
    row += 2;

#if PLATFORM_HAS_ASPECT_RATIO_OPTION
    controlList.push_back(new LegacyGuiButton(BUTTON_ASPECT_RATIO, x, legacyLayout.rowY(row++), w, h,
        settings->getKeyBinding(EnumOptions::ASPECT_RATIO)));
#endif

    legacyUiCheckbox = new LegacyOptionCheckbox(BUTTON_LEGACY_UI, x, legacyLayout.rowY(row++), w, h,
        "Legacy UI", settings->legacyUI);
    controlList.push_back(legacyUiCheckbox);

    legacyLookCheckbox = new LegacyOptionCheckbox(BUTTON_LEGACY_LOOK, x, legacyLayout.rowY(row++), w, h,
        "Legacy Look", settings->legacyLook);
    controlList.push_back(legacyLookCheckbox);

#ifdef WII_PLATFORM
    alternativeControlsCheckbox = new LegacyOptionCheckbox(BUTTON_ALTERNATIVE_CONTROLS, x,
        legacyLayout.rowY(row++), w, h, "Alternative Controls", settings->alternativeControllerLayout);
    controlList.push_back(alternativeControlsCheckbox);
#endif

#if PLATFORM_HAS_CONTROLLER_CALIBRATION
    controlList.push_back(new LegacyGuiButton(BUTTON_DEADZONE, x, legacyLayout.rowY(row++), w, h,
        "Deadzone Settings"));
#endif

    controlList.push_back(new LegacyGuiButton(BUTTON_DONE, x, legacyLayout.rowY(row), w, h, "Done"));
}

void LegacyHeritageOptions::saveIdentity()
{
    if (settings == nullptr)
        return;
    settings->playerName = sanitizeHeritagePlayerName(nameField != nullptr ? nameField->getText() : "");
    if (nameField != nullptr)
        nameField->setText(settings->playerName);
    if (mc != nullptr && mc->session != nullptr)
        mc->session->username = settings->playerName;
}

void LegacyHeritageOptions::saveAndClose()
{
    returnToParent();
}

void LegacyHeritageOptions::updateScreen()
{
    LegacyOptionsScreen::updateScreen();
    if (nameField != nullptr)
        nameField->updateCursorCounter();
}

void LegacyHeritageOptions::onGuiClosed()
{
    if (nameField != nullptr)
        nameField->setFocused(false);
}

void LegacyHeritageOptions::keyTyped(char_t c, int_t key)
{
    if (nameField != nullptr && nameField->getFocused())
    {
        if (c == '\r' || key == lwjgl::Keyboard::KEY_RETURN)
        {
            saveIdentity();
            settings->saveOptions();
            nameField->setFocused(false);
            return;
        }
        nameField->textboxKeyTyped(c, key);
        return;
    }
    if (handleLegacyNavigationKey(key))
        return;
}

void LegacyHeritageOptions::mouseClicked(int_t x, int_t y, int_t button)
{
    GuiScreen::mouseClicked(x, y, button);
    if (nameField != nullptr)
        nameField->mouseClicked(x, y, button);
}

void LegacyHeritageOptions::actionPerformed(GuiButton *button)
{
    if (button == nullptr || !button->enabled || settings == nullptr)
        return;

    if (button->id == BUTTON_EDIT_PLAYER_NAME)
    {
        if (nameField != nullptr)
            nameField->setFocused(true);
        return;
    }

    // Toggling an option may save or reconstruct the screen. Preserve the name
    // before either operation so it cannot revert to the value loaded at entry.
    saveIdentity();

#if PLATFORM_HAS_ASPECT_RATIO_OPTION
    if (button->id == BUTTON_ASPECT_RATIO)
    {
        settings->setOptionValue(EnumOptions::ASPECT_RATIO, 1);
        ScaledResolution sr(settings, mc->displayWidth, mc->displayHeight);
        setWorldAndResolution(mc, sr.getScaledWidth(), sr.getScaledHeight());
        return;
    }
#endif

    if (button->id == BUTTON_LEGACY_UI)
    {
        const int_t previousScale = settings->guiScale;
        settings->setLegacyUiEnabled(!settings->legacyUI);
        settings->saveOptions();
        if (settings->guiScale != previousScale && mc != nullptr)
        {
            ScaledResolution sr(settings, mc->displayWidth, mc->displayHeight);
            setWorldAndResolution(mc, sr.getScaledWidth(), sr.getScaledHeight());
            return;
        }
        if (legacyUiCheckbox != nullptr)
            legacyUiCheckbox->setChecked(settings->legacyUI);
        return;
    }

    if (button->id == BUTTON_LEGACY_LOOK)
    {
        settings->legacyLook = !settings->legacyLook;
        if (legacyLookCheckbox != nullptr)
            legacyLookCheckbox->setChecked(settings->legacyLook);
        settings->saveOptions();
        if (mc != nullptr && mc->entityRenderer != nullptr)
            mc->entityRenderer->updateWorldLightLevels();
        return;
    }

#ifdef WII_PLATFORM
    if (button->id == BUTTON_ALTERNATIVE_CONTROLS)
    {
        settings->alternativeControllerLayout = !settings->alternativeControllerLayout;
        PlatformUserSettings::setAlternativeControls(settings->alternativeControllerLayout);
        if (alternativeControlsCheckbox != nullptr)
            alternativeControlsCheckbox->setChecked(settings->alternativeControllerLayout);
        settings->saveOptions();
        return;
    }
#endif

#if PLATFORM_HAS_CONTROLLER_CALIBRATION
    if (button->id == BUTTON_DEADZONE)
    {
        settings->saveOptions();
        mc->displayGuiScreen(new GuiDeadzoneSettings(this, settings));
        return;
    }
#endif

    if (button->id == BUTTON_DONE)
    {
        saveAndClose();
        return;
    }
}

void LegacyHeritageOptions::returnToParent()
{
    saveIdentity();
    LegacyOptionsScreen::returnToParent();
}

void LegacyHeritageOptions::drawScreen(int_t mouseX, int_t mouseY, float_t partialTick)
{
    drawLegacyBackground(partialTick);
    legacyDrawOptionText(fontRenderer, "Player Name", legacyLayout.contentX + 2,
        legacyOptionTextY(legacyLayout.rowY(0), legacyLayout.rowHeight), legacyOptionNormalTextColor());
    if (nameField != nullptr)
        nameField->drawTextBox();
    updateLegacyPointerHover(mouseX, mouseY);
    GuiScreen::drawScreen(mouseX, mouseY, partialTick);
}
