#include "GuiSmallButton.h"
#include "EnumOptions.h"

GuiSmallButton::GuiSmallButton(int_t id, int_t x, int_t y, const std::string &text)
	// Faithful to vanilla GuiSmallButton: this(i,j,k,null,s) -> super(...,150,20,...).
	// A GuiSmallButton is ALWAYS 150x20; delegating to GuiButton(id,x,y,text) gave it
	// the 200-wide default, which stretched the Video Settings sub-screen buttons.
	: GuiSmallButton(id, x, y, nullptr, text)
{
}

GuiSmallButton::GuiSmallButton(int_t id, int_t x, int_t y, int_t w, int_t h, const std::string &text)
	: GuiButton(id, x, y, w, h, text)
	, enumOptions(nullptr)
{
}

GuiSmallButton::GuiSmallButton(int_t id, int_t x, int_t y, EnumOptions *enumOpts, const std::string &text)
	: GuiButton(id, x, y, 150, 20, text)
	, enumOptions(enumOpts)
{
}

EnumOptions *GuiSmallButton::returnEnumOptions()
{
	return enumOptions;
}
