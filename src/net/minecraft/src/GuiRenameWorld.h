#pragma once

#include "GuiScreen.h"
#include <string>

class GuiTextField;

// net.minecraft.src.GuiRenameWorld
class GuiRenameWorld : public GuiScreen
{
public:
	GuiRenameWorld(GuiScreen *parent, const std::string &saveFileName);
	~GuiRenameWorld() override;

	void updateScreen() override;
	void initGui() override;
	void onGuiClosed() override;

protected:
	void actionPerformed(GuiButton *button) override;
	void keyTyped(char_t c, int_t key) override;
	void mouseClicked(int_t x, int_t y, int_t button) override;

public:
	void drawScreen(int_t mouseX, int_t mouseY, float_t partialTick) override;

private:
	GuiScreen *parentScreen;
	GuiTextField *textbox;
	const std::string saveFileName;
};
