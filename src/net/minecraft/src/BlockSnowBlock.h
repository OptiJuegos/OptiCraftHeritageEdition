#pragma once

#include "Block.h"

class World;

// net.minecraft.src.BlockSnowBlock
class BlockSnowBlock : public Block
{
public:
	BlockSnowBlock(int_t i, int_t j);
	int_t idDropped(int_t i, Random &random) override;
	int_t quantityDropped(Random &random) override;
	void updateTick(World *world, int_t i, int_t j, int_t k, Random &random) override;
};
