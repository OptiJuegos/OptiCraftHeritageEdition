#pragma once

#include "BlockContainer.h"

class AxisAlignedBB;
class IBlockAccess;

// net.minecraft.src.BlockSign
class BlockSign : public BlockContainer
{
public:
	bool getBlocksMovement(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k) override;
	BlockSign(int_t i, TileEntity* (*entityFactory)(), bool flag);
	AxisAlignedBB *getCollisionBoundingBoxFromPool(World *world, int_t i, int_t j, int_t k) override;
	AxisAlignedBB *getSelectedBoundingBoxFromPool(World *world, int_t i, int_t j, int_t k) override;
	void setBlockBoundsBasedOnState(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k) override;
	int_t getRenderType() override;
	bool renderAsNormalBlock() override;
	bool isOpaqueCube() override;
	int_t idDropped(int_t i, Random &random) override;
	void onNeighborBlockChange(World *world, int_t i, int_t j, int_t k, int_t l) override;

protected:
	TileEntity *getBlockEntity() override;

private:
	TileEntity* (*signEntityFactory)();
	bool isFreestanding;
};
