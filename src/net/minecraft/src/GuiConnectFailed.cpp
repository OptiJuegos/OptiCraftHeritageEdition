#include "GuiConnectFailed.h"
#include "StringTranslate.h"
#include "GuiButton.h"
#include "GuiMainMenu.h"
#include "FontRenderer.h"
#include "Minecraft.h"

GuiConnectFailed::GuiConnectFailed(const std::string &errorKey, const std::string &detailKey, const std::string &arg)
{
	StringTranslate *stringtranslate = StringTranslate::getInstance();
	errorMessage = stringtranslate->translateKey(errorKey);
	if (!arg.empty())
		errorDetail = stringtranslate->translateKeyFormat(detailKey, arg.c_str());
	else
		errorDetail = stringtranslate->translateKey(detailKey);
}

void GuiConnectFailed::updateScreen()
{
}

void GuiConnectFailed::keyTyped(char_t c, int_t i)
{
}

void GuiConnectFailed::initGui()
{
	StringTranslate *stringtranslate = StringTranslate::getInstance();
	controlList.clear();
	controlList.push_back(new GuiButton(0, width / 2 - 100, height / 4 + 120 + 12, stringtranslate->translateKey("gui.toMenu")));
}

void GuiConnectFailed::actionPerformed(GuiButton *guibutton)
{
	if (guibutton->id == 0)
	{
		mc->displayGuiScreen(new GuiMainMenu());
	}
}

void GuiConnectFailed::drawScreen(int_t i, int_t j, float_t f)
{
	drawDefaultBackground();
	drawCenteredString(fontRenderer, errorMessage, width / 2, height / 2 - 50, 0xffffff);
	drawCenteredString(fontRenderer, errorDetail,  width / 2, height / 2 - 10, 0xffffff);
	GuiScreen::drawScreen(i, j, f);
}
