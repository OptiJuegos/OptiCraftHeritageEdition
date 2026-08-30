#pragma once

#include "GuiScreen.h"

// net.minecraft.src.GuiErrorScreen
class GuiErrorScreen : public GuiScreen
{
public:
	GuiErrorScreen();

	void updateScreen() override;
	void initGui() override;
	void drawScreen(int_t mouseX, int_t mouseY, float_t partialTick) override;

protected:
	void keyTyped(char_t c, int_t key) override;
	void actionPerformed(GuiButton *button) override;

private:
	int_t field_28098_a;
};
