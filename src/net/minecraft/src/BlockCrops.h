#pragma once

#include "BlockFlower.h"

// net.minecraft.src.BlockCrops
class BlockCrops : public BlockFlower
{
public:
	BlockCrops(int_t i, int_t j);
	void updateTick(World *world, int_t i, int_t j, int_t k, Random &random) override;
	void fertilize(World *world, int_t i, int_t j, int_t k);
	int_t getBlockTextureFromSideAndMetadata(int_t i, int_t j) override;
	int_t getRenderType() override;
	void dropBlockAsItemWithChance(World *world, int_t i, int_t j, int_t k, int_t l, float f) override;
	void dropBlockAsItemWithChance(World *world, int_t i, int_t j, int_t k, int_t l, float f, int_t fortune) override;
	int_t idDropped(int_t i, Random &random) override;
	int_t quantityDropped(Random &random) override;

protected:
	bool canThisPlantGrowOnThisBlockID(int_t i) override;

private:
	float getGrowthRate(World *world, int_t i, int_t j, int_t k);
};
