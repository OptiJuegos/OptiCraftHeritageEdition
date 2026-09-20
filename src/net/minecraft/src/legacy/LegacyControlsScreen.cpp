#include "net/minecraft/src/UiStrings.h"
#include "LegacyControlsScreen.h"

#include <algorithm>

#include "LegacyGuiButton.h"
#include "net/minecraft/src/FontRenderer.h"
#include "net/minecraft/src/GameSettings.h"
#include "net/minecraft/src/GuiButton.h"
#include "net/minecraft/src/GuiDeadzoneSettings.h"
#include "net/minecraft/src/Minecraft.h"
#include "pc/lwjgl/Keyboard.h"
#include "platform/Input.h"
#include "platform/PlatformConfig.h"

#if PLATFORM_PS2
#include "ps2/input/Ps2PadKeyCodes.h"
#endif

#if PLATFORM_WII
#include "wii/input/WiiPadKeyCodes.h"
#endif

namespace
{
constexpr int_t BUTTON_ROW_BASE = 7000;
constexpr int_t BUTTON_PREVIOUS = 7100;
constexpr int_t BUTTON_NEXT = 7101;
constexpr int_t BUTTON_RESET = 7102;
constexpr int_t BUTTON_BACK = 7103;
constexpr int_t BUTTON_DEADZONE = 7104;

bool reservedCaptureKey(int_t key)
{
#if PLATFORM_PS2
    return key == PS2_KEY_CIRCLE || key == lwjgl::Keyboard::KEY_ESCAPE;
#elif PLATFORM_WII
    return key == WII_KEY_GC_B || key == WII_KEY_WM_B || key == WII_KEY_CC_B ||
        key == lwjgl::Keyboard::KEY_ESCAPE;
#else
    return key == lwjgl::Keyboard::KEY_ESCAPE;
#endif
}

std::string capturePrompt()
{
#if PLATFORM_PS2 || PLATFORM_WII
    return uiText("Press a button...");
#else
    return uiText("Press a key...");
#endif
}
}

LegacyControlsScreen::LegacyControlsScreen(GuiScreen *parent, GameSettings *settingsValue,
    LegacyOptionsBackgroundMode backgroundModeValue)
    : LegacyOptionsScreen(parent, settingsValue, backgroundModeValue),
      captureRow(-1), page(0), rowsPerPage(8)
{
}

void LegacyControlsScreen::initGui()
{
    captureRow = -1;
    platformSetPadRebindExclusive(false);
    rows = legacyControlsRows(settings);
    // Reserve the footer before choosing page size, including PS2 calibration.
    const int_t footerRows = PLATFORM_PS2 ? 4 : 3;
    rowsPerPage = std::max<int_t>(PLATFORM_PS2 ? 1 : 3, std::min<int_t>(8,
        legacyOptionsMaxRows(width, height, LegacyOptionsLayoutPreset::Wide) - footerRows));
    configureLegacyLayout(rowsPerPage + footerRows, true, LegacyOptionsLayoutPreset::Wide);

    const int_t x = legacyLayout.contentX;
    const int_t w = legacyLayout.contentWidth;
    const int_t h = legacyLayout.rowHeight;

    for (int_t i = 0; i < rowsPerPage; ++i)
        controlList.push_back(new LegacyGuiButton(BUTTON_ROW_BASE + i, x, legacyLayout.rowY(i), w, h, ""));

    const int_t navY = legacyLayout.rowY(rowsPerPage);
    const int_t gap = 2;
    const int_t halfWidth = (w - gap) / 2;
    controlList.push_back(new LegacyGuiButton(BUTTON_PREVIOUS, x, navY, halfWidth, h, uiText("Previous")));
    controlList.push_back(new LegacyGuiButton(BUTTON_NEXT, x + halfWidth + gap, navY, w - halfWidth - gap, h, uiText("Next")));
    controlList.push_back(new LegacyGuiButton(BUTTON_RESET, x, legacyLayout.rowY(rowsPerPage + 1), w, h,
        uiText("Reset to Defaults")));
#if PLATFORM_PS2
    controlList.push_back(new LegacyGuiButton(BUTTON_DEADZONE, x, legacyLayout.rowY(rowsPerPage + 2), w, h,
        uiText("Deadzone Settings")));
#endif
    controlList.push_back(new LegacyGuiButton(BUTTON_BACK, x, legacyLayout.rowY(rowsPerPage + footerRows - 1), w, h, uiText("Back")));

    rebuildPage();
}

int_t LegacyControlsScreen::pageCount() const
{
    if (rowsPerPage <= 0 || rows.empty())
        return 1;
    return (static_cast<int_t>(rows.size()) + rowsPerPage - 1) / rowsPerPage;
}

void LegacyControlsScreen::refreshRowLabels()
{
    for (int_t visibleRow = 0; visibleRow < rowsPerPage; ++visibleRow)
    {
        GuiButton *button = controlList[visibleRow];
        const int_t rowIndex = page * rowsPerPage + visibleRow;
        const bool available = rowIndex >= 0 && rowIndex < static_cast<int_t>(rows.size());
        button->enabled = available;
        button->enabled2 = available;
        if (!available)
        {
            button->displayString.clear();
            continue;
        }

        const LegacyControlsBindingRow &row = rows[rowIndex];
        std::string value;
        if (captureRow == rowIndex)
            value = capturePrompt();
        else
        {
            value = legacyControlsBindingLabel(settings, row);
            if (legacyControlsBindingConflicts(settings, row))
                value = std::string("\xc2\xa7" "c") + value;
        }

        std::string label = row.label + "    " + value;
        if (fontRenderer != nullptr)
            label = fontRenderer->trimStringToWidth(label, legacyLayout.contentWidth - 8);
        button->displayString = label;
    }
}

void LegacyControlsScreen::rebuildPage()
{
    const int_t count = pageCount();
    if (page < 0)
        page = 0;
    if (page >= count)
        page = count - 1;

    refreshRowLabels();
    controlList[rowsPerPage]->enabled = page > 0;
    controlList[rowsPerPage + 1]->enabled = page + 1 < count;
    syncLegacySelection();
}

void LegacyControlsScreen::beginCapture(int_t visibleRow)
{
    const int_t rowIndex = page * rowsPerPage + visibleRow;
    if (rowIndex < 0 || rowIndex >= static_cast<int_t>(rows.size()))
        return;

    captureRow = rowIndex;
    platformSetPadRebindExclusive(true);
    refreshRowLabels();
}

void LegacyControlsScreen::cancelCapture()
{
    if (captureRow < 0)
        return;
    captureRow = -1;
    platformSetPadRebindExclusive(false);
    refreshRowLabels();
}

void LegacyControlsScreen::applyCapturedKey(int_t key)
{
    if (captureRow < 0 || captureRow >= static_cast<int_t>(rows.size()))
        return;
    if (!legacyControlsApplyCapturedKey(settings, rows[captureRow], key))
        return;

    captureRow = -1;
    platformSetPadRebindExclusive(false);
    refreshRowLabels();
}

void LegacyControlsScreen::resetDefaults()
{
    cancelCapture();
    if (settings == nullptr)
        return;
    settings->resetControlBindingsToDefaults();
    rows = legacyControlsRows(settings);
    rebuildPage();
}

void LegacyControlsScreen::actionPerformed(GuiButton *button)
{
    if (button == nullptr || !button->enabled)
        return;

    if (button->id >= BUTTON_ROW_BASE && button->id < BUTTON_ROW_BASE + rowsPerPage)
    {
        beginCapture(button->id - BUTTON_ROW_BASE);
        return;
    }
    if (button->id == BUTTON_PREVIOUS)
    {
        --page;
        rebuildPage();
        return;
    }
    if (button->id == BUTTON_NEXT)
    {
        ++page;
        rebuildPage();
        return;
    }
    if (button->id == BUTTON_RESET)
    {
        resetDefaults();
        return;
    }
#if PLATFORM_PS2
    if (button->id == BUTTON_DEADZONE)
    {
        cancelCapture();
        settings->saveOptions();
        mc->displayGuiScreen(new GuiDeadzoneSettings(this, settings));
        return;
    }
#endif
    if (button->id == BUTTON_BACK)
    {
        cancelCapture();
        returnToParent();
    }
}

void LegacyControlsScreen::keyTyped(char_t c, int_t key)
{
    if (captureRow >= 0)
    {
        if (reservedCaptureKey(key))
        {
            cancelCapture();
#if PLATFORM_PS2 || PLATFORM_WII
            // The reserved Back button is also a menu-navigation edge. Consume
            // the same latched press so it cannot immediately close Controls.
            (void)platformTextInputSnapshot(platformMenuPad());
#endif
            return;
        }
        applyCapturedKey(key);
        return;
    }
    LegacyOptionsScreen::keyTyped(c, key);
}

void LegacyControlsScreen::mouseClicked(int_t x, int_t y, int_t button)
{
#if !PLATFORM_PS2 && !PLATFORM_WII
    if (captureRow >= 0)
    {
        applyCapturedKey(-100 + button);
        return;
    }
#else
    if (captureRow >= 0)
        return;
#endif
    LegacyOptionsScreen::mouseClicked(x, y, button);
}

void LegacyControlsScreen::updateScreen()
{
    if (captureRow >= 0)
    {
        GuiScreen::updateScreen();
        syncLegacySelection();
        return;
    }
    LegacyOptionsScreen::updateScreen();
}

void LegacyControlsScreen::drawScreen(int_t mouseX, int_t mouseY, float_t partialTick)
{
    drawLegacyBackground(partialTick);
    updateLegacyPointerHover(mouseX, mouseY);
    GuiScreen::drawScreen(mouseX, mouseY, partialTick);
}

void LegacyControlsScreen::onGuiClosed()
{
    platformSetPadRebindExclusive(false);
    captureRow = -1;
}
