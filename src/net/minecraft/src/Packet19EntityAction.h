#pragma once

#include "Packet.h"

class NetHandler;
class Entity;

// net.minecraft.src.Packet19EntityAction
class Packet19EntityAction : public Packet
{
public:
	Packet19EntityAction() = default;
	Packet19EntityAction(Entity *entity, int_t state);

	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	void processPacket(NetHandler &nethandler) override;
	int_t getPacketSize() override;

	int_t entityId = 0;
	int_t state = 0;
};
