#pragma once

#include "BlockFlower.h"

// net.minecraft.src.BlockStem
class BlockStem : public BlockFlower
{
public:
	BlockStem(int_t id, Block *fruitType);

	void updateTick(World *world, int_t x, int_t y, int_t z, Random &random) override;
	void fertilizeStem(World *world, int_t x, int_t y, int_t z);
	int_t getRenderColor(int_t metadata) override;
	int_t colorMultiplier(IBlockAccess *iblockaccess, int_t x, int_t y, int_t z) override;
	int_t getBlockTextureFromSideAndMetadata(int_t side, int_t metadata) override;
	void setBlockBoundsForItemRender() override;
	void setBlockBoundsBasedOnState(IBlockAccess *iblockaccess, int_t x, int_t y, int_t z) override;
	int_t getRenderType() override;
	int_t getStateForNeighbor(IBlockAccess *iblockaccess, int_t x, int_t y, int_t z);
	void dropBlockAsItemWithChance(World *world, int_t x, int_t y, int_t z, int_t metadata, float chance, int_t fortune) override;
	int_t idDropped(int_t metadata, Random &random, int_t fortune) override;
	int_t quantityDropped(Random &random) override;

protected:
	bool canThisPlantGrowOnThisBlockID(int_t id) override;

private:
	float getGrowthModifier(World *world, int_t x, int_t y, int_t z);
	Block *fruitType;
};
