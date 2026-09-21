#include "LegacyOptionsScreen.h"

#include "LegacyMainMenuLayout.h"
#include "LegacyMenuHints.h"
#include "LegacyMenuNavigation.h"
#include "LegacyPanorama.h"
#include "LegacyPauseStyle.h"
#include "LegacyUiAssets.h"
#include "net/minecraft/src/GameSettings.h"
#include "net/minecraft/src/GuiButton.h"
#include "net/minecraft/src/Minecraft.h"
#include "net/minecraft/src/RenderEngine.h"
#include "net/minecraft/src/SoundManager.h"
#include "pc/lwjgl/Keyboard.h"
#include "platform/Input.h"
#include "platform/PlatformConfig.h"
#include "LegacySceneState.h"

LegacyOptionsScreen::LegacyOptionsScreen(GuiScreen *parent, GameSettings *settingsValue,
    LegacyOptionsBackgroundMode backgroundModeValue)
    : parentScreen(parent), settings(settingsValue), backgroundMode(backgroundModeValue),
      selectedControlIndex(-1), hoveredControlIndex(-1), panoramaAvailable(false), panelVisible(false),
      ps2ActionReleaseLatch(true)
{
}

void LegacyOptionsScreen::configureLegacyLayout(int_t rowCount, bool drawPanel, LegacyOptionsLayoutPreset preset)
{
    legacyLayout = legacyOptionsLayout(width, height, rowCount, preset);
    panelVisible = drawPanel;
    hoveredControlIndex = -1;
    panoramaAvailable = backgroundMode == LegacyOptionsBackgroundMode::Panorama &&
        mc != nullptr && mc->renderEngine != nullptr &&
        mc->renderEngine->hasResource(legacyPanoramaResourcePath());
}


void LegacyOptionsScreen::syncLegacySelection()
{
    if (controlList.empty())
    {
        selectedControlIndex = -1;
        return;
    }
    if (selectedControlIndex < 0 || selectedControlIndex >= static_cast<int_t>(controlList.size()) ||
        controlList[selectedControlIndex] == nullptr || !controlList[selectedControlIndex]->enabled ||
        !controlList[selectedControlIndex]->enabled2)
    {
        selectedControlIndex = legacyFirstSelectableButton(controlList);
    }
    legacyApplyMenuSelection(controlList, hoveredControlIndex >= 0 ? -1 : selectedControlIndex);
}

void LegacyOptionsScreen::updateLegacyPointerHover(int_t mouseX, int_t mouseY)
{
    hoveredControlIndex = legacyHoveredSelectableButton(controlList, mouseX, mouseY);
    if (hoveredControlIndex >= 0)
        selectedControlIndex = hoveredControlIndex;
    syncLegacySelection();
}

void LegacyOptionsScreen::moveLegacySelection(int_t direction)
{
    if (hoveredControlIndex >= 0)
        return;
    syncLegacySelection();
    const int_t previous = selectedControlIndex;
    selectedControlIndex = legacyNextSelectableButton(controlList, selectedControlIndex, direction);
    legacyApplyMenuSelection(controlList, selectedControlIndex);
    if (selectedControlIndex != previous && mc != nullptr && mc->sndManager != nullptr)
        mc->sndManager->playSoundFX("random.focus", 1.0f, 1.0f);
}

void LegacyOptionsScreen::activateLegacySelection()
{
    syncLegacySelection();
    const int_t targetIndex = hoveredControlIndex >= 0 ? hoveredControlIndex : selectedControlIndex;
    if (targetIndex < 0 || targetIndex >= static_cast<int_t>(controlList.size()))
        return;
    if (mc != nullptr && mc->sndManager != nullptr)
        mc->sndManager->playSoundFX("random.action", 1.0f, 1.0f);
    actionPerformed(controlList[targetIndex]);
}

void LegacyOptionsScreen::adjustLegacySelection(int_t direction)
{
    if (hoveredControlIndex >= 0)
        return;
    syncLegacySelection();
    if (selectedControlIndex < 0 || selectedControlIndex >= static_cast<int_t>(controlList.size()))
        return;
    GuiButton *button = controlList[selectedControlIndex];
    if (button != nullptr && button->adjustKeyboard(mc, direction) && mc != nullptr && mc->sndManager != nullptr)
        mc->sndManager->playSoundFX("random.focus", 1.0f, 1.0f);
}

bool LegacyOptionsScreen::handleLegacyNavigationKey(int_t key)
{
    if (key == lwjgl::Keyboard::KEY_ESCAPE)
    {
        if (mc != nullptr && mc->sndManager != nullptr)
            mc->sndManager->playSoundFX("random.back", 1.0f, 1.0f);
        returnToParent();
        return true;
    }
#if !PLATFORM_PS2 && !PLATFORM_WII
    if (key == lwjgl::Keyboard::KEY_UP)
    {
        moveLegacySelection(-1);
        return true;
    }
    if (key == lwjgl::Keyboard::KEY_DOWN)
    {
        moveLegacySelection(1);
        return true;
    }
    if (key == lwjgl::Keyboard::KEY_RETURN)
    {
        activateLegacySelection();
        return true;
    }
#endif
    return false;
}

void LegacyOptionsScreen::keyTyped(char_t c, int_t key)
{
    if (handleLegacyNavigationKey(key))
        return;
    GuiScreen::keyTyped(c, key);
}

void LegacyOptionsScreen::updateScreen()
{
    GuiScreen::updateScreen();
    syncLegacySelection();
#if PLATFORM_PS2 || PLATFORM_WII
    // A focused GuiTextField gives the virtual keyboard exclusive ownership of
    // these buttons. Do not move or activate the menu underneath the overlay.
    if (platformTextInputExclusive())
        return;

    const PlatformTextInputSnapshot pad = platformTextInputSnapshot(platformMenuPad());
#if PLATFORM_PS2
    std::uint32_t pressed = pad.pressed;
    if (ps2ActionReleaseLatch)
    {
        pressed &= ~PLATFORM_TEXT_TYPE;
        if ((pad.held & PLATFORM_TEXT_TYPE) == 0)
            ps2ActionReleaseLatch = false;
    }

    if ((pressed & (PLATFORM_TEXT_CLOSE | PLATFORM_TEXT_SHIFT)) != 0)
    {
        if (mc != nullptr && mc->sndManager != nullptr)
            mc->sndManager->playSoundFX("random.back", 1.0f, 1.0f);
        returnToParent();
        return;
    }
    if ((pressed & PLATFORM_TEXT_UP) != 0)
        moveLegacySelection(-1);
    else if ((pressed & PLATFORM_TEXT_DOWN) != 0)
        moveLegacySelection(1);
    if ((pressed & PLATFORM_TEXT_LEFT) != 0)
        adjustLegacySelection(-1);
    else if ((pressed & PLATFORM_TEXT_RIGHT) != 0)
        adjustLegacySelection(1);
    if ((pressed & PLATFORM_TEXT_TYPE) != 0)
        activateLegacySelection();
#elif PLATFORM_WII
    if (!platformMenuPointerActive())
    {
        if ((pad.pressed & PLATFORM_TEXT_LEFT) != 0)
            adjustLegacySelection(-1);
        else if ((pad.pressed & PLATFORM_TEXT_RIGHT) != 0)
            adjustLegacySelection(1);
        if ((pad.pressed & PLATFORM_TEXT_TYPE) != 0)
            activateLegacySelection();
    }
#endif
#if PLATFORM_WII
    if ((pad.pressed & PLATFORM_TEXT_BACK) != 0)
    {
        if (mc != nullptr && mc->sndManager != nullptr)
            mc->sndManager->playSoundFX("random.back", 1.0f, 1.0f);
        returnToParent();
    }
#endif
#endif
}

void LegacyOptionsScreen::returnToParent()
{
    if (settings != nullptr)
        settings->saveOptions();
    mc->displayGuiScreen(parentScreen);
}

void LegacyOptionsScreen::drawLegacyBackground(float_t partialTick)
{
    bool drewPanorama = false;
    if (backgroundMode == LegacyOptionsBackgroundMode::PausedWorld)
    {
        // Reuse the exact pause-menu overlay. The generic Java world background
        // uses a much darker 0xC0/0xD0 overlay and made the world
        // visibly darken again when entering Help & Options from pause.
        drawGradientRect(0, 0, width, height,
            legacyPauseOverlayTopColor(), legacyPauseOverlayBottomColor());
    }
    else
    {
        drewPanorama = panoramaAvailable &&
            legacyDrawPanorama(mc, width, height, legacyScenePanoramaTimer(), partialTick, zLevel);
        if (!drewPanorama)
            drawDefaultBackground();
        else
            drawGradientRect(0, 0, width, height, static_cast<int_t>(0x14000000u), static_cast<int_t>(0x3c000000u));
    }

    LegacyMainMenuLayout titleLayout{};
    titleLayout.titleY = legacyLayout.titleY;
    titleLayout.titleMaxWidth = legacyLayout.titleMaxWidth;
    titleLayout.titleMaxHeight = legacyLayout.titleMaxHeight;
    legacyDrawTitleTexture(mc, titleLayout, width, zLevel, nullptr);

    if (panelVisible)
        panelRenderer.draw(legacyLayout);

    syncLegacySelection();
    drawLegacyMenuHints(fontRenderer, width, height, true);
}
