#pragma once

#include "Packet.h"

class NetHandler;
class WorldType;

// net.minecraft.src.Packet9Respawn
class Packet9Respawn : public Packet
{
public:
	Packet9Respawn() = default;
	Packet9Respawn(int_t dimension, byte_t difficulty, WorldType *terrainType, int_t worldHeight, int_t creativeMode);

	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	void processPacket(NetHandler &nethandler) override;
	int_t getPacketSize() override;

	int_t respawnDimension = 0;
	int_t &dimension = respawnDimension; // Compatibility with existing call sites.
	int_t difficulty = 0;
	int_t worldHeight = 0;
	int_t creativeMode = 0;
	WorldType *terrainType = nullptr; // Static WorldType registry entry; not owned.
};
