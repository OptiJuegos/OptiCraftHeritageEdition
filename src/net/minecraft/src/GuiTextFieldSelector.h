#pragma once

#include "GuiButton.h"

// Transparent menu control that makes a GuiTextField part of controller/
// keyboard navigation without drawing a second widget over the field. The
// one-pixel ring is only shown while the field row is selected or hovered.
class GuiTextFieldSelector : public GuiButton
{
public:
	GuiTextFieldSelector(int_t id, int_t x, int_t y, int_t width, int_t height)
		: GuiButton(id, x, y, width, height, "")
	{
	}

	void drawButton(Minecraft *, int_t mouseX, int_t mouseY) override
	{
		if (!enabled2)
			return;
		const bool hovered = keyboardSelected ||
			(mouseX >= xPosition && mouseY >= yPosition &&
			 mouseX < xPosition + width && mouseY < yPosition + height);
		if (!hovered)
			return;

		const int_t color = enabled ? static_cast<int_t>(0xffffffA0u) : static_cast<int_t>(0xffa0a0a0u);
		drawRect(xPosition - 1, yPosition - 1, xPosition + width + 1, yPosition, color);
		drawRect(xPosition - 1, yPosition + height, xPosition + width + 1, yPosition + height + 1, color);
		drawRect(xPosition - 1, yPosition, xPosition, yPosition + height, color);
		drawRect(xPosition + width, yPosition, xPosition + width + 1, yPosition + height, color);
	}
};
