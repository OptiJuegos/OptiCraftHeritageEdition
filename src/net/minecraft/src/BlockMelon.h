#pragma once

#include "Block.h"

// net.minecraft.src.BlockMelon
class BlockMelon : public Block
{
public:
	explicit BlockMelon(int_t id);

	int_t getBlockTextureFromSideAndMetadata(int_t side, int_t metadata) override;
	int_t getBlockTextureFromSide(int_t side) override;
	int_t idDropped(int_t metadata, Random &random, int_t fortune) override;
	int_t quantityDropped(Random &random) override;
	int_t quantityDroppedWithBonus(int_t fortune, Random &random) override;
};
