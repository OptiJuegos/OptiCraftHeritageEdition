#pragma once

#include "Block.h"

class AxisAlignedBB;
class Entity;

// net.minecraft.src.BlockCactus
class BlockCactus : public Block
{
public:
	BlockCactus(int_t i, int_t j);
	void updateTick(World *world, int_t i, int_t j, int_t k, Random &random) override;
	AxisAlignedBB *getCollisionBoundingBoxFromPool(World *world, int_t i, int_t j, int_t k) override;
	AxisAlignedBB *getSelectedBoundingBoxFromPool(World *world, int_t i, int_t j, int_t k) override;
	int_t getBlockTextureFromSide(int_t i) override;
	bool renderAsNormalBlock() override;
	bool isOpaqueCube() override;
	int_t getRenderType() override;
	bool canPlaceBlockAt(World *world, int_t i, int_t j, int_t k) override;
	void onNeighborBlockChange(World *world, int_t i, int_t j, int_t k, int_t l) override;
	bool canBlockStay(World *world, int_t i, int_t j, int_t k) override;
	void onEntityCollidedWithBlock(World *world, int_t i, int_t j, int_t k, Entity *entity) override;
};
