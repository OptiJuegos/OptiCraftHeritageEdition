#pragma once

#include "GuiScreen.h"
#include <string>

class GuiTexturePackSlot;

// net.minecraft.src.GuiTexturePacks
class GuiTexturePacks : public GuiScreen
{
	friend class GuiTexturePackSlot;

public:
	GuiTexturePacks(GuiScreen *guiscreen);
	~GuiTexturePacks() override;

	void initGui() override;
	void updateScreen() override;
	void drawScreen(int_t mouseX, int_t mouseY, float_t partialTick) override;

protected:
	void actionPerformed(GuiButton *button) override;
	void mouseClicked(int_t x, int_t y, int_t button) override;
	void mouseMovedOrUp(int_t x, int_t y, int_t button) override;

protected:
	GuiScreen *guiScreen;

private:
	int_t field_6454_o;
	std::string fileLocation;
	GuiTexturePackSlot *guiTexturePackSlot;
};
