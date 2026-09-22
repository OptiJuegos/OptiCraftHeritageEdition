#pragma once

#include "GuiScreen.h"

class GuiButton;

class GuiLoadModsMenu : public GuiScreen
{
public:
    GuiLoadModsMenu(GuiScreen *parent);

    void initGui() override;
    void actionPerformed(GuiButton *button) override;
    void drawScreen(int_t mouseX, int_t mouseY, float_t partialTick) override;
    void keyTyped(char_t c, int_t key) override;
    void handleSpecializedMenuInput() override;
    bool allowsPlatformPointerInput() const override { return true; }

private:
    GuiScreen *parentScreen;
};
