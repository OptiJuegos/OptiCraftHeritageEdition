#pragma once

#include "GuiScreen.h"

// net.minecraft.src.GuiIngameMenu
class GuiIngameMenu : public GuiScreen
{
public:
	GuiIngameMenu();

	void initGui() override;

protected:
	void actionPerformed(GuiButton *button) override;

public:
	void updateScreen() override;
	void drawScreen(int_t mouseX, int_t mouseY, float_t partialTick) override;

private:
	int_t updateCounter2;
	int_t updateCounter;
};
