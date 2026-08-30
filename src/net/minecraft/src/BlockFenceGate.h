#pragma once

#include "BlockDirectional.h"

// net.minecraft.src.BlockFenceGate
class BlockFenceGate : public BlockDirectional
{
public:
	BlockFenceGate(int_t id, int_t texture);

	bool canPlaceBlockAt(World *world, int_t x, int_t y, int_t z) override;
	AxisAlignedBB *getCollisionBoundingBoxFromPool(World *world, int_t x, int_t y, int_t z) override;
	void setBlockBoundsBasedOnState(IBlockAccess *iblockaccess, int_t x, int_t y, int_t z) override;
	bool isOpaqueCube() override;
	bool renderAsNormalBlock() override;
	bool getBlocksMovement(IBlockAccess *iblockaccess, int_t x, int_t y, int_t z) override;
	int_t getRenderType() override;
	void onBlockPlacedBy(World *world, int_t x, int_t y, int_t z, EntityLiving *entityliving) override;
	bool blockActivated(World *world, int_t x, int_t y, int_t z, EntityPlayer *entityplayer) override;
	void onNeighborBlockChange(World *world, int_t x, int_t y, int_t z, int_t neighborId) override;

	static bool isFenceGateOpen(int_t metadata);
};
