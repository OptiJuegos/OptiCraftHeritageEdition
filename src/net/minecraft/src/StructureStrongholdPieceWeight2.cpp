#include "StructureStrongholdPieceWeight2.h"

StructureStrongholdPieceWeight2::StructureStrongholdPieceWeight2(
	StrongholdPieceType type, int_t weight, int_t limit)
	: StructureStrongholdPieceWeight(type, weight, limit)
{
}

bool StructureStrongholdPieceWeight2::canSpawnMoreStructuresOfType(int_t depth) const
{
	return StructureStrongholdPieceWeight::canSpawnMoreStructuresOfType(depth) && depth > 4;
}
