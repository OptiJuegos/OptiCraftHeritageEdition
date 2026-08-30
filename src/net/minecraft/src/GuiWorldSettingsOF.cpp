#include "GuiWorldSettingsOF.h"
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

GuiWorldSettingsOF::GuiWorldSettingsOF(GuiScreen *parent, GameSettings *gamesettings)
	: prevScreen(parent)
	, title("World Settings")
	, settings(gamesettings)
	, lastMouseX(0)
	, lastMouseY(0)
	, mouseStillTime(0)
{
}

void GuiWorldSettingsOF::initGui()
{
	EnumOptions *enumOptions[] = {
		EnumOptions::RENDER_DISTANCE_FINE, EnumOptions::FAR_VIEW
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

void GuiWorldSettingsOF::actionPerformed(GuiButton *button)
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

void GuiWorldSettingsOF::drawScreen(int_t mouseX, int_t mouseY, float_t partialTick)
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
	if (PlatformCompat::getTicks() < mouseStillTime + activateDelay)
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

std::string GuiWorldSettingsOF::getButtonName(const std::string &displayString)
{
	std::size_t pos = displayString.find(':');
	if (pos == std::string::npos)
		return displayString;
	return displayString.substr(0, pos);
}

GuiButton *GuiWorldSettingsOF::getSelectedButton(int_t x, int_t y)
{
	for (GuiButton *btn : controlList)
	{
		if (x >= btn->xPosition && y >= btn->yPosition &&
			x < btn->xPosition + btn->getButtonWidth() && y < btn->yPosition + btn->getButtonHeight())
			return btn;
	}
	return nullptr;
}

std::vector<std::string> GuiWorldSettingsOF::getTooltipLines(const std::string &btnName)
{
	if (btnName == "Load Far")
		return {"Loads the world chunks at distance Far.", "Switching the render distance does not cause all chunks ", "to be loaded again.", "  OFF - world chunks loaded up to render distance", "  ON - world chunks loaded at distance Far, allows", "       fast render distance switching"};
	if (btnName == "Preloaded Chunks")
		return {"Defines an area in which no chunks will be loaded", "  OFF - after 5m new chunks will be loaded", "  2 - after 32m  new chunks will be loaded", "  8 - after 128m new chunks will be loaded", "Higher values need more time to load all the chunks"};
	if (btnName == "Chunk Updates")
		return {"Chunk updates per frame", " 1 - (default) slower world loading, higher FPS", " 3 - faster world loading, lower FPS", " 5 - fastest world loading, lowest FPS"};
	if (btnName == "Dynamic Updates")
		return {"Chunk updates per frame", " OFF - (default) standard chunk updates per frame", " ON - more updates while the player is standing still", "Dynamic updates force more chunk updates while", "the player is standing still to load the world faster."};
	if (btnName == "Render Distance")
		return {"Fine render distance in 16-block steps.", "Desktop range: 32 to 512 blocks.", "Console builds are clamped to their resident chunk budget."};
	if (btnName == "Far View")
		return {"Far View", " OFF - (default) standard view distance", " ON - 3x view distance", "Far View is very resource demanding!", "3x view distance => 9x chunks to be loaded => FPS / 9", "Standard view distances: 32, 64, 128, 256", "Far view distances: 96, 192, 384, 512"};
	if (btnName == "Time")
		return {"Time", " Default - normal day/night cycles", " Day Only - day only", " Night Only - night only"};
	if (btnName == "Weather")
		return {"Weather", "  ON - weather is active, slower", "  OFF  - weather is not active, faster", "The weather controls rain, snow and thunderstorms."};
	return {};
}
