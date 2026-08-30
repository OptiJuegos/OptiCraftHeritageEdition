#include "GuiPerformanceSettingsOF.h"
#include "GuiSmallButton.h"
#include "GuiSlider.h"
#include "GuiButton.h"
#include "GameSettings.h"
#include "EnumOptions.h"
#include "StringTranslate.h"
#include "ScaledResolution.h"
#include "Minecraft.h"

GuiPerformanceSettingsOF::GuiPerformanceSettingsOF(GuiScreen *parent, GameSettings *gamesettings)
	: prevScreen(parent)
	, title("Performance Settings")
	, settings(gamesettings)
{
}

void GuiPerformanceSettingsOF::initGui()
{
	EnumOptions *enumOptions[] = {
		EnumOptions::SMOOTH_FPS, EnumOptions::SMOOTH_INPUT, EnumOptions::LOAD_FAR,
		EnumOptions::PRELOADED_CHUNKS, EnumOptions::CHUNK_UPDATES, EnumOptions::CHUNK_UPDATES_DYNAMIC
	};

	StringTranslate *tr = StringTranslate::getInstance();
	int_t i = 0;
	for (EnumOptions *opt : enumOptions)
	{
		const int_t x = width / 2 - 155 + i % 2 * 160;
		const int_t y = height / 6 + 21 * (i / 2) - 10;
		if (!opt->getEnumFloat())
			controlList.push_back(new GuiSmallButton(opt->returnEnumOrdinal(), x, y, opt, settings->getKeyBinding(opt)));
		else
			controlList.push_back(new GuiSlider(opt->returnEnumOrdinal(), x, y, opt, settings->getKeyBinding(opt), settings->getOptionFloatValue(opt)));
		++i;
	}
	controlList.push_back(new GuiButton(200, width / 2 - 100, height / 6 + 168 + 11, tr->translateKey("gui.done")));
}

void GuiPerformanceSettingsOF::actionPerformed(GuiButton *button)
{
	if (!button->enabled)
		return;

	if (button->id < 100 && dynamic_cast<GuiSmallButton *>(button) != nullptr)
	{
		settings->setOptionValue(static_cast<GuiSmallButton *>(button)->returnEnumOptions(), 1);
		button->displayString = settings->getKeyBinding(EnumOptions::getEnumOptions(button->id));
	}
	if (button->id == 200)
	{
		settings->saveOptions();
		mc->displayGuiScreen(prevScreen);
		return;
	}

	ScaledResolution resolution(mc->gameSettings, mc->displayWidth, mc->displayHeight);
	setWorldAndResolution(mc, resolution.getScaledWidth(), resolution.getScaledHeight());
}

void GuiPerformanceSettingsOF::drawScreen(int_t mouseX, int_t mouseY, float_t partialTick)
{
	drawDefaultBackground();
	drawCenteredString(fontRenderer, title, width / 2, 20, 0xffffff);
	GuiScreen::drawScreen(mouseX, mouseY, partialTick);
}
