#pragma once

#include "GuiScreen.h"
#include "EnumOptions.h"
#include <string>

class GameSettings;

// net.minecraft.src.GuiOptions
class GuiOptions : public GuiScreen
{
public:
	GuiOptions(GuiScreen *parent, GameSettings *settings);

	void initGui() override;

protected:
	void actionPerformed(GuiButton *button) override;

public:
	void drawScreen(int_t mouseX, int_t mouseY, float_t partialTick) override;

private:
	GuiScreen *parentScreen;
	GameSettings *options;

protected:
	std::string screenTitle;

};
