#pragma once

#include "LegacyOptionsScreen.h"

class GuiButton;
class GuiTextField;
class LegacyOptionCheckbox;

class LegacyHeritageOptions : public LegacyOptionsScreen
{
public:
    LegacyHeritageOptions(GuiScreen *parent, GameSettings *settings,
        LegacyOptionsBackgroundMode backgroundMode = LegacyOptionsBackgroundMode::Panorama);
    ~LegacyHeritageOptions() override;

    void initGui() override;
    void updateScreen() override;
    void onGuiClosed() override;
    void drawScreen(int_t mouseX, int_t mouseY, float_t partialTick) override;

protected:
    void keyTyped(char_t c, int_t key) override;
    void mouseClicked(int_t x, int_t y, int_t button) override;
    void actionPerformed(GuiButton *button) override;
    void returnToParent() override;

private:
    void saveIdentity();
    void saveAndClose();

    GuiTextField *nameField;
    LegacyOptionCheckbox *legacyUiCheckbox;
    LegacyOptionCheckbox *legacyLookCheckbox;
    LegacyOptionCheckbox *alternativeControlsCheckbox;
};
