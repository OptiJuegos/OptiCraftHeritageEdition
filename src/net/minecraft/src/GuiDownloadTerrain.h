#pragma once

#include "GuiScreen.h"

class NetClientHandler;

// net.minecraft.src.GuiDownloadTerrain
class GuiDownloadTerrain : public GuiScreen
{
public:
	GuiDownloadTerrain(NetClientHandler *netclienthandler);

	void initGui() override;
	void updateScreen() override;
	void drawScreen(int_t mouseX, int_t mouseY, float_t partialTick) override;

protected:
	void keyTyped(char_t c, int_t key) override;
	void actionPerformed(GuiButton *button) override;

private:
	NetClientHandler *netHandler;
	int_t updateCounter;
};
