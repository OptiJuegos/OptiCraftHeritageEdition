#pragma once

#include "Block.h"

// net.minecraft.src.BlockOre
class BlockOre : public Block
{
public:
	BlockOre(int_t i, int_t j);
	int_t idDropped(int_t i, Random &random, int_t fortune) override;
	int_t quantityDropped(Random &random) override;
	int_t quantityDroppedWithBonus(int_t fortune, Random &random) override;

protected:
	int_t damageDropped(int_t i) override;
};
