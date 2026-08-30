#pragma once

#include "Packet.h"

class NetHandler;

// net.minecraft.src.Packet100OpenWindow
class Packet100OpenWindow : public Packet
{
public:
	Packet100OpenWindow() = default;

	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	void processPacket(NetHandler &nethandler) override;
	int_t getPacketSize() override;

	int_t windowId = 0;
	int_t inventoryType = 0;
	jstring windowTitle;
	int_t slotsCount = 0;
};
