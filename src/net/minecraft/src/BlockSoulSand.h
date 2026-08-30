#pragma once

#include "Block.h"

class World;
class AxisAlignedBB;
class Entity;

// net.minecraft.src.BlockSoulSand
class BlockSoulSand : public Block
{
public:
	BlockSoulSand(int_t i, int_t j);
	AxisAlignedBB *getCollisionBoundingBoxFromPool(World *world, int_t i, int_t j, int_t k) override;
	void onEntityCollidedWithBlock(World *world, int_t i, int_t j, int_t k, Entity *entity) override;
};
