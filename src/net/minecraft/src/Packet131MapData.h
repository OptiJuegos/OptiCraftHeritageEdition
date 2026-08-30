#pragma once

#include "Packet.h"
#include <vector>

class NetHandler;

// net.minecraft.src.Packet131MapData
class Packet131MapData : public Packet
{
public:
	Packet131MapData();

	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	void processPacket(NetHandler &nethandler) override;
	int_t getPacketSize() override;

	int_t itemId = 0;
	int_t mapId = 0;
	std::vector<byte_t> data;
};
