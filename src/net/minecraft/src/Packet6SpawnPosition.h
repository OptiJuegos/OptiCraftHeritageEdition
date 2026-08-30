#pragma once

#include "Packet.h"

class NetHandler;

// net.minecraft.src.Packet6SpawnPosition
class Packet6SpawnPosition : public Packet
{
public:
	Packet6SpawnPosition() = default;

	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	void processPacket(NetHandler &nethandler) override;
	int_t getPacketSize() override;

	int_t xPosition = 0;
	int_t yPosition = 0;
	int_t zPosition = 0;
};
