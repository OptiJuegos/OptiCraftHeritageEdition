#pragma once

#include "Packet.h"

class NetHandler;
class Entity;

// net.minecraft.src.Packet28EntityVelocity
class Packet28EntityVelocity : public Packet
{
public:
	Packet28EntityVelocity() = default;
	Packet28EntityVelocity(Entity *entity);
	Packet28EntityVelocity(int_t entityId, double d, double d1, double d2);

	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	void processPacket(NetHandler &nethandler) override;
	int_t getPacketSize() override;

	int_t entityId = 0;
	int_t motionX = 0;
	int_t motionY = 0;
	int_t motionZ = 0;
};
