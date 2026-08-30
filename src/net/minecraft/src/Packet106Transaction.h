#pragma once

#include "Packet.h"

class NetHandler;

// net.minecraft.src.Packet106Transaction
class Packet106Transaction : public Packet
{
public:
	Packet106Transaction() = default;

	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	void processPacket(NetHandler &nethandler) override;
	int_t getPacketSize() override;

	Packet106Transaction(int_t windowId, short_t action, bool accepted);

	int_t windowId = 0;
	short_t action = 0;
	bool accepted = false;

	short_t &actionNumber = action;
};
