#pragma once

#include "Packet.h"

class NetHandler;

// net.minecraft.src.Packet101CloseWindow
class Packet101CloseWindow : public Packet
{
public:
	Packet101CloseWindow() = default;
	Packet101CloseWindow(int_t windowId);

	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	void processPacket(NetHandler &nethandler) override;
	int_t getPacketSize() override;

	int_t windowId = 0;
};
