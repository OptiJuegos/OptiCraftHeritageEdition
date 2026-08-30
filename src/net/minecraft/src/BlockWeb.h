#pragma once

#include "Block.h"

class World;
class Entity;
class AxisAlignedBB;

// net.minecraft.src.BlockWeb
class BlockWeb : public Block
{
public:
	BlockWeb(int_t i, int_t j);
	void onEntityCollidedWithBlock(World *world, int_t i, int_t j, int_t k, Entity *entity) override;
	bool isOpaqueCube() override;
	AxisAlignedBB *getCollisionBoundingBoxFromPool(World *world, int_t i, int_t j, int_t k) override;
	int_t getRenderType() override;
	bool renderAsNormalBlock() override;
	int_t idDropped(int_t i, Random &random) override;
};
