#pragma once

#include "Block.h"

// net.minecraft.src.BlockFire
class BlockFire : public Block
{
public:
	BlockFire(int_t i, int_t j);

	void initializeBlock() override;
	AxisAlignedBB *getCollisionBoundingBoxFromPool(World *world, int_t i, int_t j, int_t k) override;
	bool isOpaqueCube() override;
	bool renderAsNormalBlock() override;
	int_t getRenderType() override;
	int_t quantityDropped(Random &random) override;
	int_t tickRate() override;
	void updateTick(World *world, int_t i, int_t j, int_t k, Random &random) override;
	bool isCollidable() override;
	bool canBlockCatchFire(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k);
	int_t getChanceToEncourageFire(World *world, int_t i, int_t j, int_t k, int_t l);
	bool canPlaceBlockAt(World *world, int_t i, int_t j, int_t k) override;
	void onNeighborBlockChange(World *world, int_t i, int_t j, int_t k, int_t l) override;
	void onBlockAdded(World *world, int_t i, int_t j, int_t k) override;
	void randomDisplayTick(World *world, int_t i, int_t j, int_t k, Random &random) override;

private:
	void setBurnRate(int_t i, int_t j, int_t k);
	void tryToCatchBlockOnFire(World *world, int_t i, int_t j, int_t k, int_t l, Random &random, int_t i1);
	bool hasFlammableNeighbor(World *world, int_t i, int_t j, int_t k);
	int_t getChanceOfNeighborsEncouragingFire(World *world, int_t i, int_t j, int_t k);

	int_t chanceToEncourageFire[256];
	int_t abilityToCatchFire[256];
};
