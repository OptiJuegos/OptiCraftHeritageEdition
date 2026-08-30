#pragma once

#include "Packet.h"

class NetHandler;
class EntityItem;

// net.minecraft.src.Packet21PickupSpawn
class Packet21PickupSpawn : public Packet
{
public:
	Packet21PickupSpawn() = default;
	Packet21PickupSpawn(EntityItem *entityitem);

	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	void processPacket(NetHandler &nethandler) override;
	int_t getPacketSize() override;

	int_t entityId = 0;
	int_t xPosition = 0;
	int_t yPosition = 0;
	int_t zPosition = 0;
	byte_t rotation = 0;
	byte_t pitch = 0;
	byte_t roll = 0;
	int_t itemID = 0;
	int_t count = 0;
	int_t itemDamage = 0;
};
