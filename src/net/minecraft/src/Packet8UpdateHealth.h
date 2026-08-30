#pragma once

#include "Packet.h"

class NetHandler;

// net.minecraft.src.Packet8UpdateHealth
class Packet8UpdateHealth : public Packet
{
public:
	Packet8UpdateHealth() = default;

	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	void processPacket(NetHandler &nethandler) override;
	int_t getPacketSize() override;

	int_t healthMP = 0;
	int_t food = 0;
	float foodSaturation = 0.0f;
	int_t &health = healthMP;
};
