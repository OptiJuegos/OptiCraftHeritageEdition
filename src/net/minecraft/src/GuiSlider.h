#pragma once

#include "GuiButton.h"

class EnumOptions;
class Minecraft;

// net.minecraft.src.GuiSlider
class GuiSlider : public GuiButton
{
public:
	GuiSlider(int_t id, int_t x, int_t y, EnumOptions *option, const std::string &text, float_t value);

protected:
	int_t getHoverState(bool hovered) override;
	void mouseDragged(Minecraft *mc, int_t mouseX, int_t mouseY) override;

public:
	bool mousePressed(Minecraft *mc, int_t mouseX, int_t mouseY) override;
	void mouseReleased(int_t mouseX, int_t mouseY) override;

public:
	float_t sliderValue;
	bool dragging;

private:
	EnumOptions *idFloat;
};
