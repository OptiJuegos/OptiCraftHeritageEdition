#pragma once

#include "GuiChat.h"

// net.minecraft.src.GuiSleepMP
class GuiSleepMP : public GuiChat
{
public:
	GuiSleepMP();

	void initGui() override;
	void onGuiClosed() override;

protected:
	void keyTyped(char_t c, int_t key) override;

public:
	void drawScreen(int_t mouseX, int_t mouseY, float_t partialTick) override;

protected:
	void actionPerformed(GuiButton *button) override;

private:
	void stopSleeping();
};
