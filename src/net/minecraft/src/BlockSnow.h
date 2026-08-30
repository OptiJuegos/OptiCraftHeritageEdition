#pragma once

#include "Block.h"

class AxisAlignedBB;
class IBlockAccess;
class EntityPlayer;

// net.minecraft.src.BlockSnow
class BlockSnow : public Block
{
public:
	BlockSnow(int_t i, int_t j);
	AxisAlignedBB *getCollisionBoundingBoxFromPool(World *world, int_t i, int_t j, int_t k) override;
	bool isOpaqueCube() override;
	bool renderAsNormalBlock() override;
	void setBlockBoundsBasedOnState(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k) override;
	bool canPlaceBlockAt(World *world, int_t i, int_t j, int_t k) override;
	void onNeighborBlockChange(World *world, int_t i, int_t j, int_t k, int_t l) override;
	void harvestBlock(World *world, EntityPlayer *entityplayer, int_t i, int_t j, int_t k, int_t l) override;
	int_t idDropped(int_t i, Random &random) override;
	int_t quantityDropped(Random &random) override;
	void updateTick(World *world, int_t i, int_t j, int_t k, Random &random) override;
	bool shouldSideBeRendered(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k, int_t l) override;
	bool usesDefaultFaceCulling() const override { return false; }

private:
	bool getGlowing(World *world, int_t i, int_t j, int_t k);
};
