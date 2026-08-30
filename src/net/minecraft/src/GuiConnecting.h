#pragma once

#include "GuiScreen.h"
#include <string>

class Minecraft;
class NetClientHandler;
class ThreadConnectToServer;

// net.minecraft.src.GuiConnecting
class GuiConnecting : public GuiScreen
{
public:
	GuiConnecting(Minecraft *minecraft, const std::string &host, int_t port);
	~GuiConnecting() override;

	void updateScreen() override;
	void initGui() override;
	void drawScreen(int_t mouseX, int_t mouseY, float_t partialTick) override;
	static void setNetClientHandler(GuiConnecting *guiconnecting, NetClientHandler *netclienthandler);
	static NetClientHandler *getNetClientHandler(GuiConnecting *guiconnecting);
	static bool isCancelled(GuiConnecting *guiconnecting);

protected:
	void keyTyped(char_t c, int_t key) override;
	void actionPerformed(GuiButton *button) override;

	NetClientHandler *clientHandler;
	ThreadConnectToServer *connectThread;
	bool cancelled;
};
