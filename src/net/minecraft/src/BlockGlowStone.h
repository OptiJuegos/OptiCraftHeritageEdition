#pragma once

#include "Block.h"

// net.minecraft.src.BlockGlowStone
class BlockGlowStone : public Block
{
public:
	BlockGlowStone(int_t i, int_t j, Material *material);
	int_t quantityDroppedWithBonus(int_t fortune, Random &random) override;
	int_t quantityDropped(Random &random) override;
	int_t idDropped(int_t i, Random &random) override;
};
