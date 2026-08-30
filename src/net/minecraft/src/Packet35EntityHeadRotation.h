#pragma once

#include "Packet.h"

class NetHandler;

// net.minecraft.src.Packet35EntityHeadRotation
class Packet35EntityHeadRotation : public Packet
{
public:
	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	void processPacket(NetHandler &nethandler) override;
	int_t getPacketSize() override;

	int_t entityId = 0;
	byte_t headRotationYaw = 0;
};
