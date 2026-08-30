#pragma once

#include "Packet.h"

class NetHandler;

// net.minecraft.src.Packet4UpdateTime
class Packet4UpdateTime : public Packet
{
public:
	Packet4UpdateTime() = default;

	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	void processPacket(NetHandler &nethandler) override;
	int_t getPacketSize() override;

	long_t time = 0;
};
