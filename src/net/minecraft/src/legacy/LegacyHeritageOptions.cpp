#include "net/minecraft/src/UiStrings.h"
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
constexpr int_t BUTTON_SPLITSCREEN_LAYOUT = 607;
constexpr int_t BUTTON_LEGACY_CRAFTING = 608;
constexpr int_t BUTTON_LEGACY_CREATIVE = 609;
#if defined(CTR_PLATFORM)
// The face-button camera toggle lives in both options screens (this legacy
// one is the screen the 3DS actually shows; GuiOptiCraftOptions is the
// non-legacy twin). The three dual-screen HUD toggles below it and the
// auto-jump toggle beside it too.
constexpr int_t BUTTON_FACE_CAMERA = 610;
constexpr int_t BUTTON_TOUCH_MAP = 611;
constexpr int_t BUTTON_TOUCH_COORDS = 612;
constexpr int_t BUTTON_AUTO_JUMP = 613;
constexpr int_t BUTTON_POCKET_TOUCH = 614;
constexpr int_t BUTTON_TOGGLE_SHIFT = 615;
#endif

}

LegacyHeritageOptions::LegacyHeritageOptions(GuiScreen *parent, GameSettings *settingsValue,
    LegacyOptionsBackgroundMode backgroundModeValue)
    : LegacyOptionsScreen(parent, settingsValue, backgroundModeValue), nameField(nullptr), legacyUiCheckbox(nullptr),
      legacyLookCheckbox(nullptr), legacyCraftingCheckbox(nullptr), legacyCreativeCheckbox(nullptr),
      alternativeControlsCheckbox(nullptr)
#if defined(CTR_PLATFORM)
      , faceCameraCheckbox(nullptr), touchMapCheckbox(nullptr), touchCoordsCheckbox(nullptr), autoJumpCheckbox(nullptr),
      pocketTouchCheckbox(nullptr), toggleShiftCheckbox(nullptr)
#endif
{
}

LegacyHeritageOptions::~LegacyHeritageOptions()
{
    delete nameField;
    nameField = nullptr;
}

void LegacyHeritageOptions::initGui()
{
    int_t rowCount = 7; // player name label, player name field, Legacy UI, Legacy Look, Legacy Crafting, Legacy Creative, Done
#if PLATFORM_HAS_ASPECT_RATIO_OPTION
    ++rowCount;
#endif
#if PLATFORM_PS2
    ++rowCount;
#endif
#ifdef WII_PLATFORM
    ++rowCount;
#endif
#if defined(CTR_PLATFORM)
    // Face Camera/Auto Jump share one row, Touch Click/Toggle Shift share
    // the next. Touch Map and Touch Coords share rows with the legacy
    // options above.
    rowCount += 2;
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

#if defined(CTR_PLATFORM)
    // Touch Map and Touch Coords share rows with the legacy options in the
    // first column (no gap): Touch Map pairs with Legacy UI, Touch Coords
    // with Legacy Look. Face-Button Camera goes at the bottom, full width.
    constexpr int_t PAIR_GAP = 16;
    const int_t colW = std::max<int_t>(48, (w - PAIR_GAP) / 2);
#else
    // No paired touch toggles outside the 3DS: keep these rows full width.
    const int_t colW = w;
#endif

#if PLATFORM_HAS_ASPECT_RATIO_OPTION
    controlList.push_back(new LegacyGuiButton(BUTTON_ASPECT_RATIO, x, legacyLayout.rowY(row++), w, h,
        settings->getKeyBinding(EnumOptions::ASPECT_RATIO)));
#endif

#if PLATFORM_PS2
    controlList.push_back(new LegacyGuiButton(BUTTON_SPLITSCREEN_LAYOUT, x, legacyLayout.rowY(row++), w, h,
        settings->getKeyBinding(EnumOptions::SPLITSCREEN_LAYOUT)));
#endif

    legacyUiCheckbox = new LegacyOptionCheckbox(BUTTON_LEGACY_UI, x, legacyLayout.rowY(row), colW, h,
        uiText("Legacy UI"), settings->legacyUI);
    controlList.push_back(legacyUiCheckbox);
#if defined(CTR_PLATFORM)
    touchMapCheckbox = new LegacyOptionCheckbox(BUTTON_TOUCH_MAP, x + colW + PAIR_GAP,
        legacyLayout.rowY(row), colW, h, uiText("Touch Map"), settings->touchMap);
    controlList.push_back(touchMapCheckbox);
#endif
    ++row;

    legacyLookCheckbox = new LegacyOptionCheckbox(BUTTON_LEGACY_LOOK, x, legacyLayout.rowY(row), colW, h,
        uiText("Legacy Look"), settings->legacyLook);
    controlList.push_back(legacyLookCheckbox);
#if defined(CTR_PLATFORM)
    touchCoordsCheckbox = new LegacyOptionCheckbox(BUTTON_TOUCH_COORDS, x + colW + PAIR_GAP,
        legacyLayout.rowY(row), colW, h, uiText("Touch Coords"), settings->touchCoords);
    controlList.push_back(touchCoordsCheckbox);
#endif
    ++row;

    legacyCraftingCheckbox = new LegacyOptionCheckbox(BUTTON_LEGACY_CRAFTING, x, legacyLayout.rowY(row++), w, h,
        uiText("Legacy Crafting"), settings->legacyCrafting);
    controlList.push_back(legacyCraftingCheckbox);

    legacyCreativeCheckbox = new LegacyOptionCheckbox(BUTTON_LEGACY_CREATIVE, x, legacyLayout.rowY(row++), w, h,
        uiText("Legacy Creative"), settings->legacyCreative);
    controlList.push_back(legacyCreativeCheckbox);

#if defined(CTR_PLATFORM)
    // Face-Button Camera and Auto Jump share the bottom row (the label is
    // shortened to "Face Camera" so both fit the legacy half-width column).
    faceCameraCheckbox = new LegacyOptionCheckbox(BUTTON_FACE_CAMERA, x, legacyLayout.rowY(row), colW, h,
        uiText("Face Camera"), settings->faceButtonCamera);
    controlList.push_back(faceCameraCheckbox);
    autoJumpCheckbox = new LegacyOptionCheckbox(BUTTON_AUTO_JUMP, x + colW + PAIR_GAP,
        legacyLayout.rowY(row), colW, h, uiText("Auto Jump"), settings->autoJump);
    controlList.push_back(autoJumpCheckbox);
    ++row;
    pocketTouchCheckbox = new LegacyOptionCheckbox(BUTTON_POCKET_TOUCH, x, legacyLayout.rowY(row), colW, h,
        uiText("Touch Click"), settings->pocketTouch);
    controlList.push_back(pocketTouchCheckbox);
    // Toggle Shift sits under Auto Jump on the same row as Touch Click,
    // so the 3DS panel does not grow an extra row for it.
    toggleShiftCheckbox = new LegacyOptionCheckbox(BUTTON_TOGGLE_SHIFT, x + colW + PAIR_GAP, legacyLayout.rowY(row++), colW, h,
        uiText("Toggle Shift"), settings->toggleShift);
    controlList.push_back(toggleShiftCheckbox);
#endif

#ifdef WII_PLATFORM
    alternativeControlsCheckbox = new LegacyOptionCheckbox(BUTTON_ALTERNATIVE_CONTROLS, x,
        legacyLayout.rowY(row++), w, h, uiText("Alternative Controls"), settings->alternativeControllerLayout);
    controlList.push_back(alternativeControlsCheckbox);
#endif

#if PLATFORM_HAS_CONTROLLER_CALIBRATION
    controlList.push_back(new LegacyGuiButton(BUTTON_DEADZONE, x, legacyLayout.rowY(row++), w, h,
        uiText("Deadzone Settings")));
#endif

    controlList.push_back(new LegacyGuiButton(BUTTON_DONE, x, legacyLayout.rowY(row), w, h, uiText("Done")));
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

    if (button->id == BUTTON_SPLITSCREEN_LAYOUT)
    {
        settings->setOptionValue(EnumOptions::SPLITSCREEN_LAYOUT, 1);
        button->displayString = settings->getKeyBinding(EnumOptions::SPLITSCREEN_LAYOUT);
        settings->saveOptions();
        return;
    }

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

    if (button->id == BUTTON_LEGACY_CRAFTING)
    {
        settings->legacyCrafting = !settings->legacyCrafting;
        settings->applyLegacyCraftingBindings();
        if (legacyCraftingCheckbox != nullptr)
            legacyCraftingCheckbox->setChecked(settings->legacyCrafting);
        settings->saveOptions();
        return;
    }

    if (button->id == BUTTON_LEGACY_CREATIVE)
    {
        settings->legacyCreative = !settings->legacyCreative;
        if (legacyCreativeCheckbox != nullptr)
            legacyCreativeCheckbox->setChecked(settings->legacyCreative);
        settings->saveOptions();
        return;
    }

#if defined(CTR_PLATFORM)
    if (button->id == BUTTON_FACE_CAMERA)
    {
        settings->setFaceButtonCamera(!settings->faceButtonCamera);
        if (faceCameraCheckbox != nullptr)
            faceCameraCheckbox->setChecked(settings->faceButtonCamera);
        return;
    }
    if (button->id == BUTTON_TOUCH_MAP)
    {
        settings->touchMap = !settings->touchMap;
        if (touchMapCheckbox != nullptr)
            touchMapCheckbox->setChecked(settings->touchMap);
        settings->saveOptions();
        return;
    }
    if (button->id == BUTTON_TOUCH_COORDS)
    {
        settings->touchCoords = !settings->touchCoords;
        if (touchCoordsCheckbox != nullptr)
            touchCoordsCheckbox->setChecked(settings->touchCoords);
        settings->saveOptions();
        return;
    }
    if (button->id == BUTTON_AUTO_JUMP)
    {
        settings->autoJump = !settings->autoJump;
        if (autoJumpCheckbox != nullptr)
            autoJumpCheckbox->setChecked(settings->autoJump);
        settings->saveOptions();
        return;
    }
    if (button->id == BUTTON_POCKET_TOUCH)
    {
        settings->setPocketTouch(!settings->pocketTouch);
        if (pocketTouchCheckbox != nullptr)
            pocketTouchCheckbox->setChecked(settings->pocketTouch);
        return;
    }
    if (button->id == BUTTON_TOGGLE_SHIFT)
    {
        settings->toggleShift = !settings->toggleShift;
        if (toggleShiftCheckbox != nullptr)
            toggleShiftCheckbox->setChecked(settings->toggleShift);
        settings->saveOptions();
        return;
    }
#endif

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
    legacyDrawOptionText(fontRenderer, uiText("Player Name"), legacyLayout.contentX + 2,
        legacyOptionTextY(legacyLayout.rowY(0), legacyLayout.rowHeight), legacyOptionNormalTextColor());
    if (nameField != nullptr)
        nameField->drawTextBox();
    updateLegacyPointerHover(mouseX, mouseY);
    GuiScreen::drawScreen(mouseX, mouseY, partialTick);
}
