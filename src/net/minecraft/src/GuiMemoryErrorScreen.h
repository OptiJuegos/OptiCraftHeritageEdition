#pragma once

#include "GuiScreen.h"

// net.minecraft.src.GuiMemoryErrorScreen
class GuiMemoryErrorScreen : public GuiScreen
{
public:
    void updateScreen() override;
    void initGui() override;
    void drawScreen(int_t mouseX, int_t mouseY, float_t partialTick) override;

protected:
    void actionPerformed(GuiButton *button) override;
    void keyTyped(char_t c, int_t key) override;
};
