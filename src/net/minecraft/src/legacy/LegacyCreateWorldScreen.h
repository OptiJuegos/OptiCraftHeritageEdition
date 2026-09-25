#pragma once

#include "net/minecraft/src/GuiCreateWorld.h"
#include "LegacyOptionsLayout.h"
#include "LegacyOptionsPanel.h"

class LegacyDifficultySlider;
class LegacyGuiButton;

class LegacyCreateWorldScreen : public GuiCreateWorld
{
public:
    explicit LegacyCreateWorldScreen(GuiScreen *parent);

    void initGui() override;
    void updateScreen() override;
    void drawScreen(int_t mouseX, int_t mouseY, float_t partialTick) override;

protected:
    bool usesSpecializedMenuNavigation() const override { return true; }
    void actionPerformed(GuiButton *button) override;
    void keyTyped(char_t c, int_t key) override;
    void mouseClicked(int_t x, int_t y, int_t button) override;

private:
    void configureLayout();
    void drawLegacyScene(float_t partialTick);
    void drawMenuControlHints();
    void updateControlVisibility();
    void updateDifficultyControl();
    bool adjustSelection(int_t direction);
    void syncSelectedControl();
    void updatePointerHover(int_t mouseX, int_t mouseY);
    void activateSelection();
    void selectControl(int_t index);
    void moveSelection(int_t direction);
    bool isSelectionAvailable(int_t index) const;
    GuiButton *buttonForSelection(int_t index) const;
    int_t selectionForButton(const GuiButton *button) const;

    LegacyOptionsLayout layout;
    LegacyOptionsPanel panelRenderer;
    LegacyDifficultySlider *difficultySlider;
    LegacyGuiButton *legacyWorldSizeButton;
    int_t textFieldHeight;
    int_t labelOffsetY;
    int_t selectedControlIndex;
    int_t hoveredControlIndex;
    bool panoramaAvailable;
};
