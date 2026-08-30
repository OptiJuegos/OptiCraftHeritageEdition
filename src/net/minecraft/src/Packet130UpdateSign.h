#pragma once

#include "Packet.h"

class NetHandler;

// net.minecraft.src.Packet130UpdateSign
class Packet130UpdateSign : public Packet
{
public:
	Packet130UpdateSign();

	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	void processPacket(NetHandler &nethandler) override;
	int_t getPacketSize() override;

	int_t xPosition = 0;
	int_t yPosition = 0;
	int_t zPosition = 0;
	jstring signLines[4];
};
