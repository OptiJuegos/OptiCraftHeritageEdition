#pragma once

#include "Packet.h"

class NetHandler;

// net.minecraft.src.Packet39AttachEntity
class Packet39AttachEntity : public Packet
{
public:
	Packet39AttachEntity() = default;

	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	void processPacket(NetHandler &nethandler) override;
	int_t getPacketSize() override;

	int_t entityId = 0;
	int_t vehicleEntityId = 0;
};
