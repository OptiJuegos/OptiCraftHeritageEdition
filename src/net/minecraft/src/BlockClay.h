#pragma once

#include "Block.h"

class Random;

// net.minecraft.src.BlockClay
class BlockClay : public Block
{
public:
	BlockClay(int_t i, int_t j);

	int_t idDropped(int_t i, Random &random) override;
	int_t quantityDropped(Random &random) override;
};
