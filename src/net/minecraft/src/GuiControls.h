#pragma once

#include "GuiScreen.h"
#include <string>

class GameSettings;

// net.minecraft.src.GuiControls
class GuiControls : public GuiScreen
{
public:
	GuiControls(GuiScreen *parent, GameSettings *settings);

	void initGui() override;

protected:
	void actionPerformed(GuiButton *button) override;
	void keyTyped(char_t c, int_t key) override;
	void mouseClicked(int_t x, int_t y, int_t button) override;

public:
	void drawScreen(int_t mouseX, int_t mouseY, float_t partialTick) override;

private:
	int_t getLeftEdge() const;

	GuiScreen *parentScreen;
	GameSettings *options;

protected:
	std::string screenTitle;

private:
	int_t buttonId;
};
