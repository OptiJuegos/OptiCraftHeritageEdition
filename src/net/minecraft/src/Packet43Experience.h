#pragma once

#include "Packet.h"

class NetHandler;

// net.minecraft.src.Packet43Experience
class Packet43Experience : public Packet
{
public:
	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	void processPacket(NetHandler &nethandler) override;
	int_t getPacketSize() override;

	float experience = 0.0f;
	int_t experienceTotal = 0;
	int_t experienceLevel = 0;
};
