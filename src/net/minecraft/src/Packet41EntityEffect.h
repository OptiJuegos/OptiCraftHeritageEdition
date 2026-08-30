#pragma once

#include "Packet.h"

class NetHandler;

// net.minecraft.src.Packet41EntityEffect
class Packet41EntityEffect : public Packet
{
public:
	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	void processPacket(NetHandler &nethandler) override;
	int_t getPacketSize() override;

	int_t entityId = 0;
	byte_t effectId = 0;
	byte_t effectAmp = 0;
	short_t duration = 0;
};
