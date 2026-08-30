#pragma once

#include "Packet.h"

class NetHandler;

// net.minecraft.src.Packet200Statistic
class Packet200Statistic : public Packet
{
public:
	Packet200Statistic() = default;

	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	void processPacket(NetHandler &nethandler) override;
	int_t getPacketSize() override;

	int_t statId = 0;
	int_t amount = 0;
	int_t &statValue = amount;
};
