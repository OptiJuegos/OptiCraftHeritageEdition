#pragma once

#include "GuiScreen.h"

// net.minecraft.src.GuiConflictWarning
class GuiConflictWarning : public GuiScreen
{
public:
	GuiConflictWarning();

	void updateScreen() override;
	void initGui() override;
	void drawScreen(int_t mouseX, int_t mouseY, float_t partialTick) override;

protected:
	void actionPerformed(GuiButton *button) override;

private:
	int_t updateCounter;
};
