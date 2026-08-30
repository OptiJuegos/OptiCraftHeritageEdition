#include "GuiOtherSettingsOF.h"
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

GuiOtherSettingsOF::GuiOtherSettingsOF(GuiScreen *parent, GameSettings *gamesettings)
	: prevScreen(parent)
	, title("Other Settings")
	, settings(gamesettings)
	, lastMouseX(0)
	, lastMouseY(0)
	, mouseStillTime(0)
{
}

void GuiOtherSettingsOF::initGui()
{
	EnumOptions *enumOptions[] = {
		EnumOptions::FAST_DEBUG_INFO, EnumOptions::PROFILER, EnumOptions::WEATHER,
		EnumOptions::TIME, EnumOptions::AUTOSAVE_TICKS
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

void GuiOtherSettingsOF::actionPerformed(GuiButton *button)
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

void GuiOtherSettingsOF::drawScreen(int_t mouseX, int_t mouseY, float_t partialTick)
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

std::string GuiOtherSettingsOF::getButtonName(const std::string &displayString)
{
	std::size_t pos = displayString.find(':');
	if (pos == std::string::npos)
		return displayString;
	return displayString.substr(0, pos);
}

GuiButton *GuiOtherSettingsOF::getSelectedButton(int_t x, int_t y)
{
	for (GuiButton *btn : controlList)
	{
		if (x >= btn->xPosition && y >= btn->yPosition &&
			x < btn->xPosition + btn->getButtonWidth() && y < btn->yPosition + btn->getButtonHeight())
			return btn;
	}
	return nullptr;
}

std::vector<std::string> GuiOtherSettingsOF::getTooltipLines(const std::string &btnName)
{
	if (btnName == "Smooth FPS")
		return {"Stabilizes FPS by flushing the graphic driver buffers", "  OFF - no stabilization, FPS may fluctuate", "  ON - FPS stabilization", "This option is graphic driver dependant and its effect", "is not always visible"};
	if (btnName == "Smooth Input")
		return {"Fixes stuck keys, slow input response and sound lag", "  OFF - no fix for stuck keys", "  ON - fixes stuck keys", "This option adds a small delay (1ms) to the game loop", "which fixes the stuck keys, slow input and sound lag."};
	if (btnName == "Autosave")
		return {"Autosave interval", "Default autosave interval (2s) is NOT RECOMMENDED.", "Autosave causes the famous Lag Spike of Death."};
	if (btnName == "Fast Debug Info")
		return {"Fast Debug Info", " OFF - default debug info screen, slower", " ON - debug info screen without lagometer, faster", "Removes the lagometer from the debug screen (F3)."};
	return {};
}
