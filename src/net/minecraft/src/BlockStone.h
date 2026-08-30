#pragma once

#include "Block.h"

// net.minecraft.src.BlockStone
class BlockStone : public Block
{
public:
	BlockStone(int_t i, int_t j);
	int_t idDropped(int_t i, Random &random) override;
};
