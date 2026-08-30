#include "GuiErrorScreen.h"
#include "GuiButton.h"
#include "FontRenderer.h"

#include "platform/Diagnostics.h"

GuiErrorScreen::GuiErrorScreen()
	: field_28098_a(0)
{
}

void GuiErrorScreen::updateScreen()
{
	field_28098_a++;
}

void GuiErrorScreen::initGui()
{
}

void GuiErrorScreen::actionPerformed(GuiButton *guibutton)
{
}

void GuiErrorScreen::keyTyped(char_t c, int_t i)
{
}

void GuiErrorScreen::drawScreen(int_t i, int_t j, float_t f)
{
	drawDefaultBackground();
	drawCenteredString(fontRenderer, "Out of memory!",                                                  width / 2,      (height / 4 - 60) + 20,      0xffffff);
	drawString(fontRenderer,        "Minecraft has run out of memory.",                                 width / 2 - 140, (height / 4 - 60) + 60 + 0,  0xa0a0a0);
	drawString(fontRenderer,        "This could be caused by a bug in the game or by the",              width / 2 - 140, (height / 4 - 60) + 60 + 18, 0xa0a0a0);
	drawString(fontRenderer,        "Java Virtual Machine not being allocated enough",                   width / 2 - 140, (height / 4 - 60) + 60 + 27, 0xa0a0a0);
	drawString(fontRenderer,        "memory. If you are playing in a web browser, try",                  width / 2 - 140, (height / 4 - 60) + 60 + 36, 0xa0a0a0);
	drawString(fontRenderer,        "downloading the game and playing it offline.",                      width / 2 - 140, (height / 4 - 60) + 60 + 45, 0xa0a0a0);
	drawString(fontRenderer,        "To prevent level corruption, the current game has quit.",           width / 2 - 140, (height / 4 - 60) + 60 + 63, 0xa0a0a0);
	drawString(fontRenderer,        "Please restart the game.",                                          width / 2 - 140, (height / 4 - 60) + 60 + 81, 0xa0a0a0);
#ifdef WII_PLATFORM
	// This is a pre-cleanup snapshot. Minecraft::freeMemoryForCrash() has
	// already released world data by the time this screen is drawn, so querying
	// the allocator here would hide the cause rather than diagnose it.
	const int diagY = (height / 4 - 60) + 60 + 108;
	drawString(fontRenderer, platformOomDiagnosticLine(0), width / 2 - 140, diagY,      0xffff55);
	drawString(fontRenderer, platformOomDiagnosticLine(1), width / 2 - 140, diagY + 10, 0xe0e0e0);
	drawString(fontRenderer, platformOomDiagnosticLine(2), width / 2 - 140, diagY + 20, 0xe0e0e0);
	drawString(fontRenderer, platformOomDiagnosticLine(3), width / 2 - 140, diagY + 30, 0xe0e0e0);
	drawString(fontRenderer, platformOomDiagnosticLine(4), width / 2 - 140, diagY + 40, 0xe0e0e0);
#endif
	GuiScreen::drawScreen(i, j, f);
}
