#pragma once

#include "Packet.h"

class NetHandler;

// net.minecraft.src.Packet22Collect
class Packet22Collect : public Packet
{
public:
	Packet22Collect() = default;

	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	void processPacket(NetHandler &nethandler) override;
	int_t getPacketSize() override;

	int_t collectedEntityId = 0;
	int_t collectorEntityId = 0;
};
