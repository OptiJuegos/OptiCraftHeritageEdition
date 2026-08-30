#pragma once

#include "GuiScreen.h"

// net.minecraft.src.GuiGameOver
class GuiGameOver : public GuiScreen
{
public:
	GuiGameOver();

	void initGui() override;
	void drawScreen(int_t mouseX, int_t mouseY, float_t partialTick) override;
	bool doesGuiPauseGame() override;
	void updateScreen() override;

protected:
	int_t cooldownTimer;
	void keyTyped(char_t c, int_t key) override;
	void actionPerformed(GuiButton *button) override;
};
