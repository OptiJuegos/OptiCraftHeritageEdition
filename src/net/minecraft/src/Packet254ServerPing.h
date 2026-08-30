#pragma once

#include "Packet.h"

class NetHandler;

// net.minecraft.src.Packet254ServerPing
class Packet254ServerPing : public Packet
{
public:
	void readPacketData(std::istream &) override {}
	void writePacketData(std::ostream &) override {}
	void processPacket(NetHandler &nethandler) override;
	int_t getPacketSize() override { return 0; }
};
