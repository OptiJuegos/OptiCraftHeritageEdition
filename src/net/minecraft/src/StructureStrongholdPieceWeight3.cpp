#include "StructureStrongholdPieceWeight3.h"

StructureStrongholdPieceWeight3::StructureStrongholdPieceWeight3(
	StrongholdPieceType type, int_t weight, int_t limit)
	: StructureStrongholdPieceWeight(type, weight, limit)
{
}

bool StructureStrongholdPieceWeight3::canSpawnMoreStructuresOfType(int_t depth) const
{
	return StructureStrongholdPieceWeight::canSpawnMoreStructuresOfType(depth) && depth > 5;
}
