#pragma once

#include "BlockSand.h"

// net.minecraft.src.BlockGravel
class BlockGravel : public BlockSand
{
public:
	BlockGravel(int_t i, int_t j);
	int_t idDropped(int_t i, Random &random) override;
	int_t idDropped(int_t i, Random &random, int_t fortune) override;
};
