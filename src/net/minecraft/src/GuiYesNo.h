#pragma once

#include "GuiScreen.h"
#include <string>

// net.minecraft.src.GuiYesNo
class GuiYesNo : public GuiScreen
{
public:
	GuiYesNo(GuiScreen *parent, const std::string &msg1, const std::string &msg2, int_t worldNum);
	GuiYesNo(GuiScreen *parent, const std::string &msg1, const std::string &msg2,
	         const std::string &yesText, const std::string &noText, int_t worldNum);

	void initGui() override;

protected:
	void actionPerformed(GuiButton *button) override;

public:
	void drawScreen(int_t mouseX, int_t mouseY, float_t partialTick) override;

protected:
	std::string buttonText1;
	std::string buttonText2;

private:
	GuiScreen *parentScreen;
	std::string message1;
	std::string message2;
	int_t worldNumber;
};
