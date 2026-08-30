#pragma once

#include "WeightedRandomChoice.h"
#include "java/Type.h"

// net.minecraft.src.StructurePieceTreasure
class StructurePieceTreasure : public WeightedRandomChoice
{
public:
	StructurePieceTreasure(int_t itemId, int_t itemMetadata,
	                       int_t minStackSize, int_t maxStackSize, int_t weight);

	int_t itemID;
	int_t itemMetadata;
	int_t minItemStack;
	int_t maxItemStack;
};
