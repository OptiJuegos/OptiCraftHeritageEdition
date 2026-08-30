#pragma once

#include "StructureStrongholdPieceWeight.h"

// net.minecraft.src.StructureStrongholdPieceWeight2
class StructureStrongholdPieceWeight2 final : public StructureStrongholdPieceWeight
{
public:
	StructureStrongholdPieceWeight2(StrongholdPieceType type, int_t weight, int_t limit);
	bool canSpawnMoreStructuresOfType(int_t depth) const override;
};
