#pragma once

#include "Packet.h"

class NetHandler;

// net.minecraft.src.Packet10Flying
class Packet10Flying : public Packet
{
public:
	Packet10Flying() = default;
	Packet10Flying(bool onGround);

	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	void processPacket(NetHandler &nethandler) override;
	int_t getPacketSize() override;

	double xPosition = 0;
	double yPosition = 0;
	double zPosition = 0;
	double stance = 0;
	float yaw = 0;
	float pitch = 0;
	bool onGround = false;
	bool moving = false;
	bool rotating = false;
};
