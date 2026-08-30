#pragma once

#include "java/Type.h"

enum class VillagePieceType
{
	House4Garden,
	Church,
	House1,
	WoodHut,
	Hall,
	Field,
	Field2,
	House2,
	House3
};

// net.minecraft.src.StructureVillagePieceWeight
class StructureVillagePieceWeight
{
public:
	StructureVillagePieceWeight(VillagePieceType type, int_t weight, int_t limit);

	bool canSpawnMoreVillagePiecesOfType(int_t depth) const;
	bool canSpawnMoreVillagePieces() const;

	VillagePieceType villagePieceType;
	const int_t villagePieceWeight;
	int_t villagePiecesSpawned;
	int_t villagePiecesLimit;
};
