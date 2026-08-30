#pragma once

#include "Packet.h"

class NetHandler;

// net.minecraft.src.Packet7UseEntity
class Packet7UseEntity : public Packet
{
public:
	Packet7UseEntity() = default;
	Packet7UseEntity(int_t playerEntityId, int_t targetEntity, int_t isLeftClick);

	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	void processPacket(NetHandler &nethandler) override;
	int_t getPacketSize() override;

	int_t playerEntityId = 0;
	int_t targetEntity = 0;
	int_t isLeftClick = 0;
};
