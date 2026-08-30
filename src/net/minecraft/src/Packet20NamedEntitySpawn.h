#pragma once

#include "Packet.h"

class NetHandler;
class EntityPlayer;

// net.minecraft.src.Packet20NamedEntitySpawn
class Packet20NamedEntitySpawn : public Packet
{
public:
	Packet20NamedEntitySpawn() = default;
	Packet20NamedEntitySpawn(EntityPlayer *entityplayer);

	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	void processPacket(NetHandler &nethandler) override;
	int_t getPacketSize() override;

	int_t entityId = 0;
	jstring name;
	int_t xPosition = 0;
	int_t yPosition = 0;
	int_t zPosition = 0;
	byte_t rotation = 0;
	byte_t pitch = 0;
	int_t currentItem = 0;
};
