#pragma once

#include "Packet.h"

class NetHandler;

// net.minecraft.src.Packet2Handshake
class Packet2Handshake : public Packet
{
public:
	Packet2Handshake() = default;
	Packet2Handshake(const jstring &s);

	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	void processPacket(NetHandler &nethandler) override;
	int_t getPacketSize() override;

	jstring username;
};
