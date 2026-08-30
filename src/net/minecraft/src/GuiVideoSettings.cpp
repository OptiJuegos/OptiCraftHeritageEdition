#include "GuiVideoSettings.h"
#include "GuiSmallButton.h"
#include "GuiSlider.h"
#include "GuiButton.h"
#include "GameSettings.h"
#include "EnumOptions.h"
#include "StringTranslate.h"
#include "ScaledResolution.h"
#include "Minecraft.h"
#include "GuiDetailSettingsOF.h"
#include "GuiAnimationSettingsOF.h"
#include "GuiOtherSettingsOF.h"
#include "GuiTexturePacks.h"
#include "GuiPerformanceSettingsOF.h"
#include "GuiQualitySettingsOF.h"

GuiVideoSettings::GuiVideoSettings(GuiScreen *parent, GameSettings *settings)
	: screenTitle("Video Settings")
	, parentScreen(parent)
	, guiGameSettings(settings)
{
}

void GuiVideoSettings::initGui()
{
	// Build this list at use time so EnumOptions from another translation unit
	// has completed its dynamic initialization. Faithful to OptiFine nj.java's
	// option array, minus the two mipmap entries (MIPMAP_LEVEL / MIPMAP_TYPE) and
	// HD textures which are out of this port's scope. AO_LEVEL (OptiFine slider)
	// replaces the vanilla AMBIENT_OCCLUSION toggle, as in OptiFine.
	EnumOptions *shownOptions[] = {
		EnumOptions::GRAPHICS,        // k
		EnumOptions::RENDER_DISTANCE_FINE, // OptiFine fine-distance slider
		EnumOptions::AO_LEVEL,        // OptiFine smooth-lighting slider
		EnumOptions::FRAMERATE_LIMIT, // i
		EnumOptions::ANAGLYPH,        // g
		EnumOptions::VIEW_BOBBING,    // f
		EnumOptions::GUI_SCALE,       // m
		EnumOptions::ADVANCED_OPENGL, // h
		EnumOptions::FOG_FANCY,
		EnumOptions::FOG_START,
		EnumOptions::BRIGHTNESS
	};

	StringTranslate *tr = StringTranslate::getInstance();
	screenTitle = tr->translateKey("options.videoTitle");
	int_t i = 0;
	for (EnumOptions *opt : shownOptions)
	{
		int_t bx = (width / 2 - 155) + (i % 2) * 160;
		int_t by = height / 6 + 21 * (i / 2) - 10;
		if (!opt->getEnumFloat())
			controlList.push_back(new GuiSmallButton(opt->returnEnumOrdinal(), bx, by, opt, guiGameSettings->getKeyBinding(opt)));
		else
			controlList.push_back(new GuiSlider(opt->returnEnumOrdinal(), bx, by, opt, guiGameSettings->getKeyBinding(opt), guiGameSettings->getOptionFloatValue(opt)));
		i++;
	}
	// --- OptiFine: sub-screen buttons, continuing the same 21px grid (nj.java) ---
	int_t y = height / 6 + 21 * ((i + 1) / 2) - 10;
	controlList.push_back(new GuiSmallButton(100, width / 2 - 155,       y, "Animations..."));
	controlList.push_back(new GuiSmallButton(101, width / 2 - 155 + 160, y, "Details..."));
	y += 21;
	controlList.push_back(new GuiSmallButton(105, width / 2 - 155,       y, "Quality..."));
	controlList.push_back(new GuiSmallButton(104, width / 2 - 155 + 160, y, "Performance..."));
	y += 21;
	controlList.push_back(new GuiSmallButton(102, width / 2 - 155,       y, "Texture Packs..."));
	controlList.push_back(new GuiSmallButton(103, width / 2 - 155 + 160, y, "Other..."));
	controlList.push_back(new GuiButton(200, width / 2 - 100, height / 6 + 168 + 11, tr->translateKey("gui.done")));
}

void GuiVideoSettings::actionPerformed(GuiButton *button)
{
	if (!button->enabled) return;

	const int_t previousGuiScale = guiGameSettings->guiScale;
	if (button->id < 100 && dynamic_cast<GuiSmallButton *>(button))
	{
		guiGameSettings->setOptionValue(static_cast<GuiSmallButton *>(button)->returnEnumOptions(), 1);
		button->displayString = guiGameSettings->getKeyBinding(EnumOptions::getEnumOptions(button->id));
	}
	if (button->id == 200)
	{
		mc->gameSettings->saveOptions();
		mc->displayGuiScreen(parentScreen);
		return;
	}
	// --- OptiFine: open sub-screens (ids 100-103, matching nj.java) ---
	if (button->id == 100)
	{
		mc->gameSettings->saveOptions();
		mc->displayGuiScreen(new GuiAnimationSettingsOF(this, guiGameSettings));
		return;
	}
	if (button->id == 101)
	{
		mc->gameSettings->saveOptions();
		mc->displayGuiScreen(new GuiDetailSettingsOF(this, guiGameSettings));
		return;
	}
	if (button->id == 102)
	{
		mc->gameSettings->saveOptions();
		mc->displayGuiScreen(new GuiTexturePacks(this));
		return;
	}
	if (button->id == 103)
	{
		mc->gameSettings->saveOptions();
		mc->displayGuiScreen(new GuiOtherSettingsOF(this, guiGameSettings));
		return;
	}
	if (button->id == 104)
	{
		mc->gameSettings->saveOptions();
		mc->displayGuiScreen(new GuiPerformanceSettingsOF(this, guiGameSettings));
		return;
	}
	if (button->id == 105)
	{
		mc->gameSettings->saveOptions();
		mc->displayGuiScreen(new GuiQualitySettingsOF(this, guiGameSettings));
		return;
	}
	// Brightness and AO Level sliders must NOT rebuild the resolution (nj.java).
	if (button->id == EnumOptions::BRIGHTNESS->returnEnumOrdinal() ||
	    button->id == EnumOptions::AO_LEVEL->returnEnumOrdinal())
		return;
	if (guiGameSettings->guiScale != previousGuiScale)
	{
		ScaledResolution sr(mc->gameSettings, mc->displayWidth, mc->displayHeight);
		setWorldAndResolution(mc, sr.getScaledWidth(), sr.getScaledHeight());
	}
}

void GuiVideoSettings::drawScreen(int_t mouseX, int_t mouseY, float_t partialTick)
{
	drawDefaultBackground();
	drawCenteredString(fontRenderer, screenTitle, width / 2, 20, 0xffffff);
	GuiScreen::drawScreen(mouseX, mouseY, partialTick);
}
