#pragma once

#include "Block.h"

class World;
class AxisAlignedBB;

// net.minecraft.src.BlockFlower
class BlockFlower : public Block
{
public:
	BlockFlower(int_t i, int_t j);

	bool canPlaceBlockAt(World *world, int_t i, int_t j, int_t k) override;
	void onNeighborBlockChange(World *world, int_t i, int_t j, int_t k, int_t l) override;
	void updateTick(World *world, int_t i, int_t j, int_t k, Random &random) override;
	bool canBlockStay(World *world, int_t i, int_t j, int_t k) override;
	AxisAlignedBB *getCollisionBoundingBoxFromPool(World *world, int_t i, int_t j, int_t k) override;
	bool isOpaqueCube() override;
	bool renderAsNormalBlock() override;
	int_t getRenderType() override;

protected:
	BlockFlower(int_t i, int_t j, Material *material);
	virtual bool canThisPlantGrowOnThisBlockID(int_t i);
	void getPlantColor(World *world, int_t i, int_t j, int_t k);
};
