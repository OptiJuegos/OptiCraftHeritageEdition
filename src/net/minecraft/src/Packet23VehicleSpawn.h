#pragma once

#include "Packet.h"

class NetHandler;

// net.minecraft.src.Packet23VehicleSpawn
class Packet23VehicleSpawn : public Packet
{
public:
	Packet23VehicleSpawn() = default;

	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	void processPacket(NetHandler &nethandler) override;
	int_t getPacketSize() override;

	int_t entityId = 0;
	int_t xPosition = 0;
	int_t yPosition = 0;
	int_t zPosition = 0;
	int_t speedX = 0;      // blockMaterial
	int_t speedY = 0;      // blockHardness
	int_t speedZ = 0;      // blockResistance
	int_t type = 0;
	int_t throwerEntityId = 0; // blockLightValue

	int_t &velocityX = speedX;
	int_t &velocityY = speedY;
	int_t &velocityZ = speedZ;
};
