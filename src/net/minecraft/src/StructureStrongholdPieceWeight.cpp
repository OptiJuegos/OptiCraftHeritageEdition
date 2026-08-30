#include "StructureStrongholdPieceWeight.h"

StructureStrongholdPieceWeight::StructureStrongholdPieceWeight(
	StrongholdPieceType type, int_t weight, int_t limit)
	: pieceType(type), pieceWeight(weight), instancesSpawned(0), instancesLimit(limit)
{
}

bool StructureStrongholdPieceWeight::canSpawnMoreStructuresOfType(int_t) const
{
	return instancesLimit == 0 || instancesSpawned < instancesLimit;
}

bool StructureStrongholdPieceWeight::canSpawnMoreStructures() const
{
	return instancesLimit == 0 || instancesSpawned < instancesLimit;
}
