#pragma once

#include "Packet.h"

class NetHandler;

// net.minecraft.src.Packet201PlayerInfo
class Packet201PlayerInfo : public Packet
{
public:
	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	void processPacket(NetHandler &nethandler) override;
	int_t getPacketSize() override;

	jstring playerName;
	bool isConnected = false;
	int_t ping = 0;
};
