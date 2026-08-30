#pragma once

#include "BlockFlower.h"

// net.minecraft.src.BlockNetherStalk
class BlockNetherStalk : public BlockFlower
{
public:
	explicit BlockNetherStalk(int_t id);

	bool canBlockStay(World *world, int_t x, int_t y, int_t z) override;
	void updateTick(World *world, int_t x, int_t y, int_t z, Random &random) override;
	int_t getBlockTextureFromSideAndMetadata(int_t side, int_t metadata) override;
	int_t getRenderType() override;
	void dropBlockAsItemWithChance(World *world, int_t x, int_t y, int_t z, int_t metadata, float chance, int_t fortune) override;
	int_t idDropped(int_t metadata, Random &random, int_t fortune) override;
	int_t quantityDropped(Random &random) override;

protected:
	bool canThisPlantGrowOnThisBlockID(int_t id) override;
};
