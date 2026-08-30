#pragma once

#include "Block.h"

class AxisAlignedBB;

// net.minecraft.src.BlockLadder
class BlockLadder : public Block
{
public:
	BlockLadder(int_t i, int_t j);
	AxisAlignedBB *getCollisionBoundingBoxFromPool(World *world, int_t i, int_t j, int_t k) override;
	AxisAlignedBB *getSelectedBoundingBoxFromPool(World *world, int_t i, int_t j, int_t k) override;
	bool isOpaqueCube() override;
	bool renderAsNormalBlock() override;
	int_t getRenderType() override;
	bool canPlaceBlockAt(World *world, int_t i, int_t j, int_t k) override;
	void onBlockPlaced(World *world, int_t i, int_t j, int_t k, int_t l) override;
	void onNeighborBlockChange(World *world, int_t i, int_t j, int_t k, int_t l) override;
	int_t quantityDropped(Random &random) override;
};
