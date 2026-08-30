#pragma once

#include "GuiScreen.h"
#include <string>

// net.minecraft.src.GuiConnectFailed
class GuiConnectFailed : public GuiScreen
{
public:
	GuiConnectFailed(const std::string &errorKey, const std::string &detailKey, const std::string &arg = {});

	void updateScreen() override;
	void initGui() override;
	void drawScreen(int_t mouseX, int_t mouseY, float_t partialTick) override;

protected:
	void keyTyped(char_t c, int_t key) override;
	void actionPerformed(GuiButton *button) override;

private:
	std::string errorMessage;
	std::string errorDetail;
};
