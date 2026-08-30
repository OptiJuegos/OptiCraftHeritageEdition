#pragma once

#include "GuiButton.h"

class EnumOptions;

// net.minecraft.src.GuiSmallButton
class GuiSmallButton : public GuiButton
{
public:
	GuiSmallButton(int_t id, int_t x, int_t y, const std::string &text);
	GuiSmallButton(int_t id, int_t x, int_t y, int_t w, int_t h, const std::string &text);
	GuiSmallButton(int_t id, int_t x, int_t y, EnumOptions *enumOptions, const std::string &text);

	EnumOptions *returnEnumOptions();

private:
	EnumOptions *enumOptions;
};
