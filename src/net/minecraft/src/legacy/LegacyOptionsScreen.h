#pragma once

#include "net/minecraft/src/GuiScreen.h"
#include "LegacyOptionsLayout.h"
#include "LegacyOptionsPanel.h"

class GameSettings;

enum class LegacyOptionsBackgroundMode
{
    Panorama,
    PausedWorld
};

class LegacyOptionsScreen : public GuiScreen
{
public:
    LegacyOptionsScreen(GuiScreen *parent, GameSettings *settings,
        LegacyOptionsBackgroundMode backgroundMode = LegacyOptionsBackgroundMode::Panorama);

    void updateScreen() override;

protected:
    bool usesSpecializedMenuNavigation() const override { return true; }
    void configureLegacyLayout(int_t rowCount, bool drawPanel,
        LegacyOptionsLayoutPreset preset = LegacyOptionsLayoutPreset::Wide);
    void keyTyped(char_t c, int_t key) override;
    bool handleLegacyNavigationKey(int_t key);
    void activateLegacySelection();
    void adjustLegacySelection(int_t direction);
    void moveLegacySelection(int_t direction);
    void syncLegacySelection();
    void updateLegacyPointerHover(int_t mouseX, int_t mouseY);
    void drawLegacyBackground(float_t partialTick);
    virtual void returnToParent();

    GuiScreen *parentScreen;
    GameSettings *settings;
    LegacyOptionsLayout legacyLayout;
    LegacyOptionsBackgroundMode backgroundMode;
    int_t selectedControlIndex;
    int_t hoveredControlIndex;

private:
    bool panoramaAvailable;
    bool panelVisible;
    bool ps2ActionReleaseLatch;
    LegacyOptionsPanel panelRenderer;
};
