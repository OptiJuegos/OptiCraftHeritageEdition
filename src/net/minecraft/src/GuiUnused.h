#pragma once

#include "GuiScreen.h"
#include <string>

// net.minecraft.src.GuiUnused
class GuiUnused : public GuiScreen
{
public:
	void initGui() override;
	void drawScreen(int_t mouseX, int_t mouseY, float_t partialTick) override;

protected:
	void keyTyped(char_t c, int_t key) override;

private:
	std::string message1;
	std::string message2;
};
