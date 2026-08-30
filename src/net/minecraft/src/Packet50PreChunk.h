#pragma once

#include "Packet.h"

class NetHandler;

// net.minecraft.src.Packet50PreChunk
class Packet50PreChunk : public Packet
{
public:
	Packet50PreChunk();

	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	void processPacket(NetHandler &nethandler) override;
	int_t getPacketSize() override;

	int_t xPosition = 0;
	int_t yPosition = 0;
	bool mode = false;
};
