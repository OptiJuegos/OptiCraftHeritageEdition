#include "net/minecraft/src/UiStrings.h"
#include "GuiOptions.h"
#include <algorithm>
#include "GuiSmallButton.h"
#include "GuiSlider.h"
#include "GuiButton.h"
#include "GuiVideoSettings.h"
#include "GuiControls.h"
#include "GuiLanguage.h"
#include "GuiTexturePacks.h"
#include "mods/GuiMods.h"
#include "GuiOptiCraftOptions.h"
#include "GameSettings.h"
#include "StringTranslate.h"
#include "Minecraft.h"
#include "World.h"
#include "WorldInfo.h"
#include "platform/PlatformConfig.h"

GuiOptions::GuiOptions(GuiScreen *parent, GameSettings *settings)
	: screenTitle(uiText("Options"))
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
	const int_t left = width / 2 - 155;
	const int_t top = height / 6 + 90;
	controlList.push_back(new GuiButton(101, left, top, 150, 20, tr->translateKey("options.video")));
	controlList.push_back(new GuiButton(100, left + 160, top, 150, 20, tr->translateKey("options.controls")));
	controlList.push_back(new GuiButton(103, left, top + 24, 150, 20, tr->translateKey("options.language")));
	controlList.push_back(new GuiButton(102, left + 160, top + 24, 150, 20, PLATFORM_PS2 ? uiText("Game Options...") : uiText("OptiCraft Options...")));
	controlList.push_back(new GuiButton(104, left, top + 48, 150, 20, uiText("Mods")));
	controlList.push_back(new GuiButton(105, left + 160, top + 48, 150, 20, uiText("Texture Packs")));
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
	if (button->id == 104 || button->id == 105)
	{
		options->saveOptions();
		if (button->id == 104)
			mc->displayGuiScreen(new GuiMods(this));
		else
			mc->displayGuiScreen(new GuiTexturePacks(this));
		return;
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
