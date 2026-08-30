#pragma once

#include "Packet.h"
#include <vector>

class NetHandler;

// net.minecraft.src.Packet52MultiBlockChange
class Packet52MultiBlockChange : public Packet
{
public:
	Packet52MultiBlockChange();

	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	void processPacket(NetHandler &nethandler) override;
	int_t getPacketSize() override;

	int_t xPosition = 0;
	int_t zPosition = 0;
	std::vector<byte_t> metadataArray;
	int_t size = 0;
};
