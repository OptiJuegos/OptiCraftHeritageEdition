#pragma once

#include "GuiScreen.h"
#include <string>
#include <vector>
#include <cstdint>

class GameSettings;

// net.minecraft.src.GuiOtherSettingsOF (OptiFine)
class GuiOtherSettingsOF : public GuiScreen
{
public:
	GuiOtherSettingsOF(GuiScreen *parent, GameSettings *gamesettings);

	void initGui() override;

protected:
	void actionPerformed(GuiButton *button) override;

public:
	void drawScreen(int_t mouseX, int_t mouseY, float_t partialTick) override;

private:
	std::vector<std::string> getTooltipLines(const std::string &btnName);
	std::string getButtonName(const std::string &displayString);
	GuiButton *getSelectedButton(int_t x, int_t y);

	GuiScreen *prevScreen;
	std::string title;
	GameSettings *settings;
	int_t lastMouseX;
	int_t lastMouseY;
	uint32_t mouseStillTime;
};
