#pragma once

#include "Packet.h"

class NetHandler;

// net.minecraft.src.Packet17Sleep
class Packet17Sleep : public Packet
{
public:
	Packet17Sleep() = default;

	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	void processPacket(NetHandler &nethandler) override;
	int_t getPacketSize() override;

	int_t entityId = 0;          // buttonFacing
	int_t sleepDirection = 0;    // platePressed
	int_t bedX = 0;              // buttonPressed
	int_t bedY = 0;              // plateWeight
	int_t bedZ = 0;              // plateMaterial

	int_t &sleepState = sleepDirection;
};
