#include "StructureNetherBridgePieceWeight.h"

StructureNetherBridgePieceWeight::StructureNetherBridgePieceWeight(
	NetherBridgePieceType type, int_t pieceWeight, int_t maximumCount, bool allowConsecutive)
	: pieceType(type), weight(pieceWeight), spawnedCount(0), maxCount(maximumCount), allowInRow(allowConsecutive)
{
}

bool StructureNetherBridgePieceWeight::canSpawnMoreStructuresOfType(int_t) const
{
	return maxCount == 0 || spawnedCount < maxCount;
}

bool StructureNetherBridgePieceWeight::canSpawnMoreStructures() const
{
	return maxCount == 0 || spawnedCount < maxCount;
}
