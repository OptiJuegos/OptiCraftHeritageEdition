#pragma once

#include "Block.h"

// net.minecraft.src.BlockOreStorage
class BlockOreStorage : public Block
{
public:
	BlockOreStorage(int_t i, int_t j);
	int_t getBlockTextureFromSide(int_t i) override;
};
