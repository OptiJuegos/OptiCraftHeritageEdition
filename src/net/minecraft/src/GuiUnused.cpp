#include "GuiUnused.h"
#include "FontRenderer.h"

void GuiUnused::initGui()
{
}

void GuiUnused::drawScreen(int_t i, int_t j, float_t f)
{
	drawGradientRect(0, 0, width, height, 0xff402020, 0xff501010);
	drawCenteredString(fontRenderer, message1, width / 2, 90,  0xffffff);
	drawCenteredString(fontRenderer, message2, width / 2, 110, 0xffffff);
	GuiScreen::drawScreen(i, j, f);
}

void GuiUnused::keyTyped(char_t c, int_t i)
{
}
