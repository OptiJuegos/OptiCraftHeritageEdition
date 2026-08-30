#pragma once

#include "Packet.h"

class NetHandler;

// net.minecraft.src.Packet16BlockItemSwitch
class Packet16BlockItemSwitch : public Packet
{
public:
	Packet16BlockItemSwitch() = default;
	Packet16BlockItemSwitch(int_t i);

	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	void processPacket(NetHandler &nethandler) override;
	int_t getPacketSize() override;

	int_t id = 0;
};
