#include "GuiAnimationSettingsOF.h"
#include "GuiSmallButton.h"
#include "GuiSlider.h"
#include "GuiButton.h"
#include "GameSettings.h"
#include "EnumOptions.h"
#include "StringTranslate.h"
#include "ScaledResolution.h"
#include "Minecraft.h"

GuiAnimationSettingsOF::GuiAnimationSettingsOF(GuiScreen *parent, GameSettings *gamesettings)
	: title("Animation Settings")
	, prevScreen(parent)
	, settings(gamesettings)
{
}

void GuiAnimationSettingsOF::initGui()
{
	EnumOptions *enumOptions[] = {
		EnumOptions::ANIMATED_WATER, EnumOptions::ANIMATED_LAVA, EnumOptions::ANIMATED_FIRE,
		EnumOptions::ANIMATED_PORTAL, EnumOptions::ANIMATED_REDSTONE, EnumOptions::ANIMATED_EXPLOSION,
		EnumOptions::ANIMATED_FLAME, EnumOptions::ANIMATED_SMOKE, EnumOptions::VOID_PARTICLES,
		EnumOptions::WATER_PARTICLES, EnumOptions::RAIN_SPLASH, EnumOptions::PORTAL_PARTICLES,
		EnumOptions::PARTICLES, EnumOptions::DRIPPING_WATER_LAVA, EnumOptions::ANIMATED_TERRAIN,
		EnumOptions::ANIMATED_ITEMS, EnumOptions::ANIMATED_TEXTURES
	};

	StringTranslate *tr = StringTranslate::getInstance();
	int_t i = 0;
	for (EnumOptions *opt : enumOptions)
	{
		int_t x = width / 2 - 155 + i % 2 * 160;
		int_t y = height / 6 + 21 * (i / 2) - 10;
		if (!opt->getEnumFloat())
			controlList.push_back(new GuiSmallButton(opt->returnEnumOrdinal(), x, y, opt, settings->getKeyBinding(opt)));
		else
			controlList.push_back(new GuiSlider(opt->returnEnumOrdinal(), x, y, opt, settings->getKeyBinding(opt), settings->getOptionFloatValue(opt)));
		i++;
	}
	const int_t footerY = height / 6 + 168 + 11;
	controlList.push_back(new GuiButton(210, width / 2 - 155, footerY, 70, 20, "All ON"));
	controlList.push_back(new GuiButton(211, width / 2 - 75, footerY, 70, 20, "All OFF"));
	controlList.push_back(new GuiButton(200, width / 2 + 5, footerY, 150, 20, tr->translateKey("gui.done")));
}

void GuiAnimationSettingsOF::actionPerformed(GuiButton *button)
{
	if (!button->enabled) return;

	if (button->id < 100 && dynamic_cast<GuiSmallButton *>(button))
	{
		settings->setOptionValue(static_cast<GuiSmallButton *>(button)->returnEnumOptions(), 1);
		button->displayString = settings->getKeyBinding(EnumOptions::getEnumOptions(button->id));
	}
	if (button->id == 200)
	{
		mc->gameSettings->saveOptions();
		mc->displayGuiScreen(prevScreen);
		return;
	}
	if (button->id == 210)
		settings->setAllAnimations(true);
	if (button->id == 211)
		settings->setAllAnimations(false);
	if (button->id != EnumOptions::CLOUD_HEIGHT->returnEnumOrdinal())
	{
		ScaledResolution sr(mc->gameSettings, mc->displayWidth, mc->displayHeight);
		setWorldAndResolution(mc, sr.getScaledWidth(), sr.getScaledHeight());
	}
}

void GuiAnimationSettingsOF::drawScreen(int_t mouseX, int_t mouseY, float_t partialTick)
{
	drawDefaultBackground();
	drawCenteredString(fontRenderer, title, width / 2, 20, 0xffffff);
	GuiScreen::drawScreen(mouseX, mouseY, partialTick);
}
