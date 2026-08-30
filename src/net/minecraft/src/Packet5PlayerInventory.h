#pragma once

#include "Packet.h"

class NetHandler;

// net.minecraft.src.Packet5PlayerInventory
class Packet5PlayerInventory : public Packet
{
public:
	Packet5PlayerInventory() = default;

	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	void processPacket(NetHandler &nethandler) override;
	int_t getPacketSize() override;

	int_t entityID = 0;
	int_t slot = 0;
	int_t itemID = 0;
	int_t itemDamage = 0;
};
