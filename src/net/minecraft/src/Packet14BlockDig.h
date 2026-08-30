#pragma once

#include "Packet.h"

class NetHandler;

// net.minecraft.src.Packet14BlockDig
class Packet14BlockDig : public Packet
{
public:
	Packet14BlockDig() = default;
	Packet14BlockDig(int_t status, int_t x, int_t y, int_t z, int_t face);

	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	void processPacket(NetHandler &nethandler) override;
	int_t getPacketSize() override;

	int_t xPosition = 0;
	int_t yPosition = 0;
	int_t zPosition = 0;
	int_t face = 0;
	int_t status = 0;
};
