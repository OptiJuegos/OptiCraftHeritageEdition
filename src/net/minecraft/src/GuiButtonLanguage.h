#pragma once

#include "GuiButton.h"

// net.minecraft.src.GuiButtonLanguage
class GuiButtonLanguage : public GuiButton
{
public:
    GuiButtonLanguage(int_t id, int_t x, int_t y);
    void drawButton(Minecraft *mc, int_t mouseX, int_t mouseY) override;
};
