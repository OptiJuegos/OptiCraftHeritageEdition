#pragma once

#include "Packet.h"

class NetHandler;
class Entity;

// net.minecraft.src.Packet18Animation
class Packet18Animation : public Packet
{
public:
	Packet18Animation() = default;
	Packet18Animation(Entity *entity, int_t animate);

	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	void processPacket(NetHandler &nethandler) override;
	int_t getPacketSize() override;

	int_t entityId = 0;
	int_t animate = 0;
};
