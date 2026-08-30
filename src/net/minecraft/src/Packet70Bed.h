#pragma once

#include "Packet.h"

class NetHandler;

// net.minecraft.src.Packet70Bed
class Packet70Bed : public Packet
{
public:
	Packet70Bed() = default;

	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	void processPacket(NetHandler &nethandler) override;
	int_t getPacketSize() override;

	static const jstring bedMessages[];    // enumValues
	static const int_t   bedMessageCount; // size of bedMessages array
	int_t bedState = 0;                        // enumMapping
	int_t gameMode = 0;

	int_t &eventType = bedState;
};
