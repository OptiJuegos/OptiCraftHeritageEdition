#pragma once

#include "Packet.h"

class NetHandler;

// net.minecraft.src.Packet30Entity
class Packet30Entity : public Packet
{
public:
	Packet30Entity();

	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	void processPacket(NetHandler &nethandler) override;
	int_t getPacketSize() override;

	int_t entityId = 0;
	byte_t xPosition = 0;
	byte_t yPosition = 0;
	byte_t zPosition = 0;
	byte_t yaw = 0;
	byte_t pitch = 0;
	bool rotating = false;
};
