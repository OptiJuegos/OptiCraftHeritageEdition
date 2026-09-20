#pragma once

#include "legacy/LegacyOptionsScreen.h"

// Reuse the console menus' selection, confirm and back handling.
class GuiCredits : public LegacyOptionsScreen
{
public:
    GuiCredits(GuiScreen *parent, GameSettings *settings);
    void initGui() override;
    void drawScreen(int_t mouseX, int_t mouseY, float_t partialTick) override;

protected:
    void actionPerformed(GuiButton *button) override;
    void returnToParent() override;
};
