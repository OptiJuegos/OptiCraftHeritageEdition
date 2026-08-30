#pragma once

#include "BlockStone.h"

// net.minecraft.src.BlockObsidian
class BlockObsidian : public BlockStone
{
public:
	BlockObsidian(int_t i, int_t j);
	int_t quantityDropped(Random &random) override;
	int_t idDropped(int_t i, Random &random) override;
};
