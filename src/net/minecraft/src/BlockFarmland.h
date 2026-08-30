#pragma once

#include "Block.h"

class AxisAlignedBB;
class Entity;

// net.minecraft.src.BlockFarmland
class BlockFarmland : public Block
{
public:
	BlockFarmland(int_t i);
	AxisAlignedBB *getCollisionBoundingBoxFromPool(World *world, int_t i, int_t j, int_t k) override;
	bool isOpaqueCube() override;
	bool renderAsNormalBlock() override;
	int_t getBlockTextureFromSideAndMetadata(int_t i, int_t j) override;
	void updateTick(World *world, int_t i, int_t j, int_t k, Random &random) override;
	void onFallenUpon(World *world, int_t i, int_t j, int_t k, Entity *entity, float fallDistance) override;
	void onEntityWalking(World *world, int_t i, int_t j, int_t k, Entity *entity) override;
	void onNeighborBlockChange(World *world, int_t i, int_t j, int_t k, int_t l) override;
	int_t idDropped(int_t i, Random &random) override;
	int_t idDropped(int_t i, Random &random, int_t fortune) override;

private:
	bool isCropsNearby(World *world, int_t i, int_t j, int_t k);
	bool isWaterNearby(World *world, int_t i, int_t j, int_t k);
};
