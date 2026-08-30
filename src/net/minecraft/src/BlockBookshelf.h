#pragma once

#include "Block.h"

// net.minecraft.src.BlockBookshelf
class BlockBookshelf : public Block
{
public:
	BlockBookshelf(int_t i, int_t j);
	int_t getBlockTextureFromSide(int_t i) override;
	int_t quantityDropped(Random &random) override;
	int_t idDropped(int_t metadata, Random &random, int_t fortune) override;
};
