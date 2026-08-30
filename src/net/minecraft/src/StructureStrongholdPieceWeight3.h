#pragma once

#include "StructureStrongholdPieceWeight.h"

// net.minecraft.src.StructureStrongholdPieceWeight3
class StructureStrongholdPieceWeight3 final : public StructureStrongholdPieceWeight
{
public:
	StructureStrongholdPieceWeight3(StrongholdPieceType type, int_t weight, int_t limit);
	bool canSpawnMoreStructuresOfType(int_t depth) const override;
};
