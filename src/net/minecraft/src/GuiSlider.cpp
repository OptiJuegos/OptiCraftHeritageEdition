#include "GuiSlider.h"
#include "EnumOptions.h"
#include "GameSettings.h"
#include "Minecraft.h"
#include "platform/RenderAPI.h"

GuiSlider::GuiSlider(int_t id_, int_t x, int_t y, EnumOptions *option, const std::string &text, float_t value)
	: GuiButton(id_, x, y, 150, 20, text)
	, sliderValue(value)
	, dragging(false)
	, idFloat(option)
{
}

int_t GuiSlider::getHoverState(bool hovered)
{
	return 0;
}

void GuiSlider::mouseDragged(Minecraft *mc, int_t mouseX, int_t mouseY)
{
	if (!enabled2) return;

	if (dragging)
	{
		sliderValue = (float_t)(mouseX - (xPosition + 4)) / (float_t)(width - 8);
		if (sliderValue < 0.0f) sliderValue = 0.0f;
		if (sliderValue > 1.0f) sliderValue = 1.0f;
		mc->gameSettings->setOptionFloatValue(idFloat, sliderValue);
		displayString = mc->gameSettings->getKeyBinding(idFloat);
	}
	renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	drawTexturedModalRect(xPosition + (int_t)(sliderValue * (float_t)(width - 8)),     yPosition, 0,   66, 4, 20);
	drawTexturedModalRect(xPosition + (int_t)(sliderValue * (float_t)(width - 8)) + 4, yPosition, 196, 66, 4, 20);
}

bool GuiSlider::mousePressed(Minecraft *mc, int_t mouseX, int_t mouseY)
{
	if (GuiButton::mousePressed(mc, mouseX, mouseY))
	{
		sliderValue = (float_t)(mouseX - (xPosition + 4)) / (float_t)(width - 8);
		if (sliderValue < 0.0f) sliderValue = 0.0f;
		if (sliderValue > 1.0f) sliderValue = 1.0f;
		mc->gameSettings->setOptionFloatValue(idFloat, sliderValue);
		displayString = mc->gameSettings->getKeyBinding(idFloat);
		dragging = true;
		return true;
	}
	return false;
}

void GuiSlider::mouseReleased(int_t mouseX, int_t mouseY)
{
	dragging = false;
}
