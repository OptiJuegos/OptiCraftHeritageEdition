#pragma once

#include "Packet.h"
#include "java/HashSet.h"
#include "ChunkPosition.h"

class NetHandler;

// net.minecraft.src.Packet60Explosion
class Packet60Explosion : public Packet
{
public:
	Packet60Explosion() = default;

	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	void processPacket(NetHandler &nethandler) override;
	int_t getPacketSize() override;

	double explosionX = 0;
	double explosionY = 0;
	double explosionZ = 0;
	float explosionSize = 0;
	struct PositionHash
	{
		std::size_t operator()(const ChunkPosition &position) const
		{
			return static_cast<std::size_t>(static_cast<uint_t>(position.hashCode()));
		}
	};
	struct PositionEqual
	{
		bool operator()(const ChunkPosition &a, const ChunkPosition &b) const { return a.equals(b); }
	};
	JavaHashSet<ChunkPosition, PositionHash, PositionEqual> destroyedBlockPositions;
};
