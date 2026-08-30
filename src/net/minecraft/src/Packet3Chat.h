#pragma once

#include "Packet.h"

class NetHandler;

// net.minecraft.src.Packet3Chat
class Packet3Chat : public Packet
{
public:
	Packet3Chat() = default;
	Packet3Chat(const jstring &s);

	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	void processPacket(NetHandler &nethandler) override;
	int_t getPacketSize() override;

	jstring message;
};
