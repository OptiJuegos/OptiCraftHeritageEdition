#pragma once

#include "Packet.h"

class NetHandler;

// net.minecraft.src.Packet38EntityStatus
class Packet38EntityStatus : public Packet
{
public:
	Packet38EntityStatus() = default;

	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	void processPacket(NetHandler &nethandler) override;
	int_t getPacketSize() override;

	int_t entityId = 0;
	byte_t entityStatus = 0;
};
