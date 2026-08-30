#pragma once

#include "Block.h"

class AxisAlignedBB;

// net.minecraft.src.BlockReed
class BlockReed : public Block
{
public:
	BlockReed(int_t i, int_t j);
	void updateTick(World *world, int_t i, int_t j, int_t k, Random &random) override;
	bool canPlaceBlockAt(World *world, int_t i, int_t j, int_t k) override;
	void onNeighborBlockChange(World *world, int_t i, int_t j, int_t k, int_t l) override;
	bool canBlockStay(World *world, int_t i, int_t j, int_t k) override;
	AxisAlignedBB *getCollisionBoundingBoxFromPool(World *world, int_t i, int_t j, int_t k) override;
	int_t idDropped(int_t i, Random &random) override;
	bool isOpaqueCube() override;
	bool renderAsNormalBlock() override;
	int_t getRenderType() override;

protected:
	void checkBlockCoordValid(World *world, int_t i, int_t j, int_t k);
};
