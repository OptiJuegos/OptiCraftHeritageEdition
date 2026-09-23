#include "net/minecraft/src/UiStrings.h"
#include "GuiQualitySettingsOF.h"
#include "GuiSmallButton.h"
#include "GuiSlider.h"
#include "GuiButton.h"
#include "GameSettings.h"
#include "EnumOptions.h"
#include "StringTranslate.h"
#include "ScaledResolution.h"
#include "Minecraft.h"
#include "platform/RenderAPI.h"
#include "platform/PlatformConfig.h"

GuiQualitySettingsOF::GuiQualitySettingsOF(GuiScreen *parent, GameSettings *gamesettings)
	: prevScreen(parent)
	, title(uiText("Quality Settings"))
	, settings(gamesettings)
{
}

void GuiQualitySettingsOF::initGui()
{
    title = uiText("Quality Settings");
	EnumOptions *enumOptions[] = {
		EnumOptions::CLEAR_WATER, EnumOptions::RANDOM_MOBS, EnumOptions::BETTER_GRASS, EnumOptions::BETTER_SNOW,
		EnumOptions::CUSTOM_COLORS, EnumOptions::SWAMP_COLORS, EnumOptions::SMOOTH_BIOMES,
		EnumOptions::CONNECTED_TEXTURES, EnumOptions::NATURAL_TEXTURES, EnumOptions::MIPMAP_LEVEL,
		EnumOptions::MIPMAP_TYPE, EnumOptions::AF_LEVEL, EnumOptions::AA_LEVEL, EnumOptions::CUSTOM_FONTS
	};

	StringTranslate *tr = StringTranslate::getInstance();
	int_t i = 0;
	for (EnumOptions *opt : enumOptions)
	{
		if (opt == EnumOptions::RANDOM_MOBS && !PLATFORM_OPTIFINE_RANDOM_MOBS)
			continue;
		if (opt == EnumOptions::CUSTOM_FONTS && !PLATFORM_OPTIFINE_CUSTOM_FONTS)
			continue;
		if ((opt == EnumOptions::MIPMAP_LEVEL || opt == EnumOptions::MIPMAP_TYPE) &&
		    !renderSupportsFeature(RenderFeature::Mipmaps))
			continue;
		if (opt == EnumOptions::AF_LEVEL &&
		    !renderSupportsFeature(RenderFeature::AnisotropicFiltering))
			continue;
		if (opt == EnumOptions::AA_LEVEL &&
		    !renderSupportsFeature(RenderFeature::MultisampleAntialiasing))
			continue;

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

void GuiQualitySettingsOF::actionPerformed(GuiButton *button)
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

void GuiQualitySettingsOF::drawScreen(int_t mouseX, int_t mouseY, float_t partialTick)
{
	drawDefaultBackground();
	drawCenteredString(fontRenderer, title, width / 2, 20, 0xffffff);
	GuiScreen::drawScreen(mouseX, mouseY, partialTick);
}
