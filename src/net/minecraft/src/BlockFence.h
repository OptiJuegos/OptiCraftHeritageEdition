#pragma once

#include "Block.h"

class AxisAlignedBB;

// net.minecraft.src.BlockFence
class BlockFence : public Block
{
public:
	BlockFence(int_t id, int_t texture);
	BlockFence(int_t id, int_t texture, Material *material);

	bool canPlaceBlockAt(World *world, int_t x, int_t y, int_t z) override;
	AxisAlignedBB *getCollisionBoundingBoxFromPool(World *world, int_t x, int_t y, int_t z) override;
	void setBlockBoundsBasedOnState(IBlockAccess *iblockaccess, int_t x, int_t y, int_t z) override;
	bool isOpaqueCube() override;
	bool renderAsNormalBlock() override;
	bool getBlocksMovement(IBlockAccess *iblockaccess, int_t x, int_t y, int_t z) override;
	int_t getRenderType() override;
	bool canConnectFenceTo(IBlockAccess *iblockaccess, int_t x, int_t y, int_t z);
};
