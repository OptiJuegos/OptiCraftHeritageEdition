#pragma once

#include "java/Type.h"

enum class NetherBridgePieceType
{
	Straight,
	Crossing3,
	Crossing,
	Stairs,
	Throne,
	Entrance,
	Corridor5,
	Crossing2,
	Corridor2,
	Corridor,
	Corridor3,
	Corridor4,
	NetherStalkRoom
};

// net.minecraft.src.StructureNetherBridgePieceWeight
class StructureNetherBridgePieceWeight
{
public:
	StructureNetherBridgePieceWeight(NetherBridgePieceType type, int_t weight,
	                                 int_t maxCount, bool allowInRow = false);

	bool canSpawnMoreStructuresOfType(int_t depth) const;
	bool canSpawnMoreStructures() const;

	NetherBridgePieceType pieceType;
	const int_t weight;
	int_t spawnedCount;
	int_t maxCount;
	bool allowInRow;
};
