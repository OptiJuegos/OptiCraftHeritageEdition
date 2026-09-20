#pragma once

#include "GuiScreen.h"

class GameSettings;
class GuiTextField;

class GuiOptiCraftOptions : public GuiScreen
{
public:
	GuiOptiCraftOptions(GuiScreen *parent, GameSettings *settings);
	~GuiOptiCraftOptions() override;

	void initGui() override;
	void updateScreen() override;
	void onGuiClosed() override;
	void drawScreen(int_t mouseX, int_t mouseY, float_t partialTick) override;

protected:
	void keyTyped(char_t c, int_t key) override;
	void mouseClicked(int_t x, int_t y, int_t button) override;
	void actionPerformed(GuiButton *button) override;

private:
	void saveIdentity();
	void saveAndClose();
	static std::string sanitizeName(const std::string &name);

	GuiScreen *parentScreen;
	GameSettings *settings;
	GuiTextField *nameField;
};
