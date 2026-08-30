#include "StructurePieceTreasure.h"

StructurePieceTreasure::StructurePieceTreasure(int_t itemId, int_t metadata,
                                               int_t minStackSize, int_t maxStackSize,
                                               int_t weight)
	: WeightedRandomChoice(weight),
	  itemID(itemId),
	  itemMetadata(metadata),
	  minItemStack(minStackSize),
	  maxItemStack(maxStackSize)
{
}
