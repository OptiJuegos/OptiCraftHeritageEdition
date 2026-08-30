#pragma once

#include <vector>

#include "Packet.h"

class NetHandler;

// net.minecraft.src.Packet250CustomPayload
class Packet250CustomPayload : public Packet
{
public:
	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	void processPacket(NetHandler &nethandler) override;
	int_t getPacketSize() override;

	jstring channel;
	int_t length = 0;
	std::vector<byte_t> data;
};
