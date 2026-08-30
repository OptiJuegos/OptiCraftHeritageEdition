#pragma once

#include "GuiScreen.h"
#include <string>

class GameSettings;

class GuiQualitySettingsOF : public GuiScreen
{
public:
	GuiQualitySettingsOF(GuiScreen *parent, GameSettings *gamesettings);

	void initGui() override;

protected:
	void actionPerformed(GuiButton *button) override;

public:
	void drawScreen(int_t mouseX, int_t mouseY, float_t partialTick) override;

private:
	GuiScreen *prevScreen;
	std::string title;
	GameSettings *settings;
};
