#pragma once

#include "Packet.h"

class NetHandler;

// net.minecraft.src.Packet61DoorChange
class Packet61DoorChange : public Packet
{
public:
	Packet61DoorChange() = default;

	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	void processPacket(NetHandler &nethandler) override;
	int_t getPacketSize() override;

	int_t soundType = 0;    // blockRenderType  (door type)
	int_t soundData = 0;    // blockTexture     (door state)
	int_t xPosition = 0;    // blockTickOnLoad
	int_t yPosition = 0;    // blockLightOpacity
	int_t zPosition = 0;    // blockStepSound

	int_t &x        = xPosition;
	int_t &y        = yPosition;
	int_t &z        = zPosition;
	int_t &doorType  = soundType;
	int_t &doorState = soundData;
};
