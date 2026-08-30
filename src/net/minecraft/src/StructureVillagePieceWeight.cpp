#include "StructureVillagePieceWeight.h"

StructureVillagePieceWeight::StructureVillagePieceWeight(
	VillagePieceType type, int_t weight, int_t limit)
	: villagePieceType(type), villagePieceWeight(weight), villagePiecesSpawned(0), villagePiecesLimit(limit)
{
}

bool StructureVillagePieceWeight::canSpawnMoreVillagePiecesOfType(int_t) const
{
	return villagePiecesLimit == 0 || villagePiecesSpawned < villagePiecesLimit;
}

bool StructureVillagePieceWeight::canSpawnMoreVillagePieces() const
{
	return villagePiecesLimit == 0 || villagePiecesSpawned < villagePiecesLimit;
}
