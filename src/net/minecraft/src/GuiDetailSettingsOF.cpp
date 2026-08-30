#include "GuiDetailSettingsOF.h"
#include "GuiSmallButton.h"
#include "GuiSlider.h"
#include "GuiButton.h"
#include "GameSettings.h"
#include "EnumOptions.h"
#include "FontRenderer.h"
#include "StringTranslate.h"
#include "ScaledResolution.h"
#include "Minecraft.h"
#include <cstdlib>
#include "platform/PlatformCompat.h"

GuiDetailSettingsOF::GuiDetailSettingsOF(GuiScreen *parent, GameSettings *gamesettings)
	: prevScreen(parent)
	, title("Detail Settings")
	, settings(gamesettings)
	, lastMouseX(0)
	, lastMouseY(0)
	, mouseStillTime(0)
{
}

void GuiDetailSettingsOF::initGui()
{
	EnumOptions *enumOptions[] = {
		EnumOptions::CLOUDS, EnumOptions::CLOUD_HEIGHT, EnumOptions::TREES, EnumOptions::GRASS,
		EnumOptions::WATER, EnumOptions::RAIN, EnumOptions::SKY, EnumOptions::STARS,
		EnumOptions::SUN_MOON, EnumOptions::DEPTH_FOG
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
	controlList.push_back(new GuiButton(200, width / 2 - 100, height / 6 + 168 + 11, tr->translateKey("gui.done")));
}

void GuiDetailSettingsOF::actionPerformed(GuiButton *button)
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
	}
	if (button->id != EnumOptions::CLOUD_HEIGHT->returnEnumOrdinal())
	{
		ScaledResolution sr(mc->gameSettings, mc->displayWidth, mc->displayHeight);
		setWorldAndResolution(mc, sr.getScaledWidth(), sr.getScaledHeight());
	}
}

void GuiDetailSettingsOF::drawScreen(int_t mouseX, int_t mouseY, float_t partialTick)
{
	drawDefaultBackground();
	drawCenteredString(fontRenderer, title, width / 2, 20, 0xffffff);
	GuiScreen::drawScreen(mouseX, mouseY, partialTick);

	if (std::abs(mouseX - lastMouseX) > 5 || std::abs(mouseY - lastMouseY) > 5)
	{
		lastMouseX = mouseX;
		lastMouseY = mouseY;
		mouseStillTime = PlatformCompat::getTicks();
		return;
	}
	const uint32_t activateDelay = 700;
	const uint32_t currentTicks = PlatformCompat::getTicks();
	if (currentTicks < mouseStillTime + activateDelay)
		return;

	int_t x1 = width / 2 - 150;
	int_t y1 = height / 6 - 5;
	if (mouseY <= y1 + 98)
		y1 += 105;
	int_t x2 = x1 + 150 + 150;
	int_t y2 = y1 + 84 + 10;
	GuiButton *btn = getSelectedButton(mouseX, mouseY);
	if (btn != nullptr)
	{
		std::vector<std::string> lines = getTooltipLines(getButtonName(btn->displayString));
		if (lines.empty())
			return;
		drawGradientRect(x1, y1, x2, y2, -536870912, -536870912);
		for (size_t i = 0; i < lines.size(); i++)
			fontRenderer->drawString(lines[i], x1 + 5, y1 + 5 + (int_t)i * 11, 14540253);
	}
}

std::string GuiDetailSettingsOF::getButtonName(const std::string &displayString)
{
	std::size_t pos = displayString.find(':');
	if (pos == std::string::npos)
		return displayString;
	return displayString.substr(0, pos);
}

GuiButton *GuiDetailSettingsOF::getSelectedButton(int_t x, int_t y)
{
	for (GuiButton *btn : controlList)
	{
		if (x >= btn->xPosition && y >= btn->yPosition &&
			x < btn->xPosition + btn->getButtonWidth() && y < btn->yPosition + btn->getButtonHeight())
			return btn;
	}
	return nullptr;
}

std::vector<std::string> GuiDetailSettingsOF::getTooltipLines(const std::string &btnName)
{
	if (btnName == "Clouds")
		return {"Clouds", "  Default - as set by setting Graphics", "  Fast - lower quality, faster", "  Fancy - higher quality, slower", "  OFF - no clouds, fastest", "Fast clouds are rendered 2D.", "Fancy clouds are rendered 3D."};
	if (btnName == "Cloud Height")
		return {"Cloud Height", "  OFF - default height", "  100% - above world height limit"};
	if (btnName == "Trees")
		return {"Trees", "  Default - as set by setting Graphics", "  Fast - lower quality, faster", "  Fancy - higher quality, slower", "Fast trees have opaque leaves.", "Fancy trees have transparent leaves."};
	if (btnName == "Grass")
		return {"Grass", "  Default - as set by setting Graphics", "  Fast - lower quality, faster", "  Fancy - higher quality, slower", "Fast grass uses default side texture.", "Fancy grass uses biome side texture."};
	if (btnName == "Water")
		return {"Water", "  Default - as set by setting Graphics", "  Fast  - lower quality, faster", "  Fancy - higher quality, slower", "Fast water (1 pass) has some visual artifacts", "Fancy water (2 pass) has no visual artifacts"};
	if (btnName == "Rain & Snow")
		return {"Rain & Snow", "  Default - as set by setting Graphics", "  Fast  - light rain/snow, faster", "  Fancy - heavy rain/snow, slower", "  OFF - no rain/snow, fastest", "When rain is OFF the splashes and rain sounds", "are still active."};
	if (btnName == "Sky")
		return {"Sky", "  ON - sky is visible, slower", "  OFF  - sky is not visible, faster", "When sky is OFF the moon and sun are still visible."};
	if (btnName == "Stars")
		return {"Stars", "  ON - stars are visible, slower", "  OFF  - stars are not visible, faster"};
	return {};
}
