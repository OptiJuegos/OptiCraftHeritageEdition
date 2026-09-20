#include "GuiOptions.h"
#include <algorithm>
#include "GuiSmallButton.h"
#include "GuiSlider.h"
#include "GuiButton.h"
#include "GuiVideoSettings.h"
#include "GuiControls.h"
#include "GuiLanguage.h"
#include "GuiOptiCraftOptions.h"
#include "GameSettings.h"
#include "StringTranslate.h"
#include "Minecraft.h"
#include "World.h"
#include "WorldInfo.h"
#include "platform/PlatformConfig.h"

GuiOptions::GuiOptions(GuiScreen *parent, GameSettings *settings)
	: screenTitle("Options")
	, parentScreen(parent)
	, options(settings)
{
}

void GuiOptions::initGui()
{
	// Avoid cross-translation-unit static initialization order dependencies.
	EnumOptions *shownOptions[] = {
		EnumOptions::MUSIC,
		EnumOptions::SOUND,
		EnumOptions::INVERT_MOUSE,
		EnumOptions::SENSITIVITY,
		EnumOptions::FOV,
		EnumOptions::DIFFICULTY
	};

	StringTranslate *tr = StringTranslate::getInstance();
	screenTitle = tr->translateKey("options.title");
	int_t i = 0;
	for (EnumOptions *opt : shownOptions)
	{
		int_t bx = (width / 2 - 155) + (i % 2) * 160;
		int_t by = height / 6 + 24 * (i >> 1);
		if (!opt->getEnumFloat())
		{
			GuiSmallButton *optionButton = new GuiSmallButton(opt->returnEnumOrdinal(), bx, by, opt, options->getKeyBinding(opt));
			if (opt == EnumOptions::DIFFICULTY && mc != nullptr && mc->theWorld != nullptr &&
			    mc->theWorld->getWorldInfo() != nullptr && mc->theWorld->getWorldInfo()->isHardcoreModeEnabled())
			{
				optionButton->enabled = false;
				optionButton->displayString = tr->translateKey("options.difficulty") + ": " + tr->translateKey("options.difficulty.hardcore");
			}
			controlList.push_back(optionButton);
		}
		else
		{
			controlList.push_back(new GuiSlider(opt->returnEnumOrdinal(), bx, by, opt, options->getKeyBinding(opt), options->getOptionFloatValue(opt)));
		}
		i++;
	}
	controlList.push_back(new GuiButton(101, width / 2 - 100, height / 6 + 90, tr->translateKey("options.video")));
	controlList.push_back(new GuiButton(100, width / 2 - 100, height / 6 + 114, tr->translateKey("options.controls")));
	controlList.push_back(new GuiButton(103, width / 2 - 100, height / 6 + 138, 98, 20, tr->translateKey("options.language")));
	controlList.push_back(new GuiButton(102, width / 2 + 2, height / 6 + 138, 98, 20, PLATFORM_PS2 ? "Game Options..." : "OptiCraft Options..."));
	controlList.push_back(new GuiButton(200, width / 2 - 100, PLATFORM_PS2 ? std::min(height - 22, height / 6 + 168) : height / 6 + 168, tr->translateKey("gui.done")));
}

void GuiOptions::actionPerformed(GuiButton *button)
{
	if (!button->enabled) return;

	if (button->id < 100 && dynamic_cast<GuiSmallButton *>(button))
	{
		options->setOptionValue(static_cast<GuiSmallButton *>(button)->returnEnumOptions(), 1);
		button->displayString = options->getKeyBinding(EnumOptions::getEnumOptions(button->id));
	}
	if (button->id == 101)
	{
		mc->gameSettings->saveOptions();
		mc->displayGuiScreen(new GuiVideoSettings(this, options));
	}
	if (button->id == 100)
	{
		mc->gameSettings->saveOptions();
		mc->displayGuiScreen(new GuiControls(this, options));
	}
	if (button->id == 103)
	{
		mc->gameSettings->saveOptions();
		mc->displayGuiScreen(new GuiLanguage(this, options));
	}
	if (button->id == 102)
	{
		mc->gameSettings->saveOptions();
		mc->displayGuiScreen(new GuiOptiCraftOptions(this, options));
	}
	if (button->id == 200)
	{
		mc->gameSettings->saveOptions();
		mc->displayGuiScreen(parentScreen);
	}
}

void GuiOptions::drawScreen(int_t mouseX, int_t mouseY, float_t partialTick)
{
	drawDefaultBackground();
	drawCenteredString(fontRenderer, screenTitle, width / 2, 20, 0xffffff);
	GuiScreen::drawScreen(mouseX, mouseY, partialTick);
}
