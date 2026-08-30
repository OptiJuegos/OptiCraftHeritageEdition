#pragma once

#include "BlockFlower.h"

// net.minecraft.src.BlockSapling
class BlockSapling : public BlockFlower
{
public:
	BlockSapling(int_t i, int_t j);
	void updateTick(World *world, int_t i, int_t j, int_t k, Random &random) override;
	int_t getBlockTextureFromSideAndMetadata(int_t i, int_t j) override;
	void growTree(World *world, int_t i, int_t j, int_t k, Random &random);
	bool isSameSapling(World *world, int_t i, int_t j, int_t k, int_t metadata);

protected:
	int_t damageDropped(int_t i) override;
};
