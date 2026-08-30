#pragma once

#include "Packet.h"

class NetHandler;

// net.minecraft.src.Packet255KickDisconnect
class Packet255KickDisconnect : public Packet
{
public:
	Packet255KickDisconnect() = default;
	Packet255KickDisconnect(const jstring &reason);

	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	void processPacket(NetHandler &nethandler) override;
	int_t getPacketSize() override;

	jstring reason;
};
