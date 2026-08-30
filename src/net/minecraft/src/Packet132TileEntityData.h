#pragma once

#include "Packet.h"

class NetHandler;

// net.minecraft.src.Packet132TileEntityData
class Packet132TileEntityData : public Packet
{
public:
	Packet132TileEntityData();

	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	void processPacket(NetHandler &nethandler) override;
	int_t getPacketSize() override;

	int_t xPosition = 0;
	int_t yPosition = 0;
	int_t zPosition = 0;
	int_t actionType = 0;
	int_t customParam1 = 0;
	int_t customParam2 = 0;
	int_t customParam3 = 0;
};
