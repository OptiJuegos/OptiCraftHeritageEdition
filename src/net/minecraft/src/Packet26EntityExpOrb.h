#pragma once

#include "Packet.h"

class EntityXPOrb;
class NetHandler;

// net.minecraft.src.Packet26EntityExpOrb
class Packet26EntityExpOrb : public Packet
{
public:
	Packet26EntityExpOrb() = default;
	explicit Packet26EntityExpOrb(EntityXPOrb *orb);

	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	void processPacket(NetHandler &nethandler) override;
	int_t getPacketSize() override;

	int_t entityId = 0;
	int_t posX = 0;
	int_t posY = 0;
	int_t posZ = 0;
	int_t xpValue = 0;
};
