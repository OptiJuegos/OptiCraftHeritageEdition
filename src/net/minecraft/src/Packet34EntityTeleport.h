#pragma once

#include "Packet.h"

class NetHandler;
class Entity;

// net.minecraft.src.Packet34EntityTeleport
class Packet34EntityTeleport : public Packet
{
public:
	Packet34EntityTeleport() = default;
	Packet34EntityTeleport(Entity *entity);

	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	void processPacket(NetHandler &nethandler) override;
	int_t getPacketSize() override;

	int_t entityId = 0;
	int_t xPosition = 0;
	int_t yPosition = 0;
	int_t zPosition = 0;
	byte_t yaw = 0;
	byte_t pitch = 0;
};
