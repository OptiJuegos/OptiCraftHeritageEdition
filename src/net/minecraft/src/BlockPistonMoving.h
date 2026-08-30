#pragma once

#include "BlockContainer.h"

class EntityPlayer;
class IBlockAccess;
class TileEntityPiston;

// net.minecraft.src.BlockPistonMoving
class BlockPistonMoving : public BlockContainer
{
public:
	BlockPistonMoving(int_t i);

	void onBlockAdded(World *world, int_t i, int_t j, int_t k) override;
	void onBlockRemoval(World *world, int_t i, int_t j, int_t k) override;
	bool canPlaceBlockAt(World *world, int_t i, int_t j, int_t k) override;
	bool canPlaceBlockOnSide(World *world, int_t i, int_t j, int_t k, int_t l) override;
	int_t getRenderType() override;
	bool isOpaqueCube() override;
	bool renderAsNormalBlock() override;
	bool blockActivated(World *world, int_t i, int_t j, int_t k, EntityPlayer *entityplayer) override;
	int_t idDropped(int_t i, Random &random) override;
	void dropBlockAsItemWithChance(World *world, int_t i, int_t j, int_t k, int_t l, float f) override;
	void onNeighborBlockChange(World *world, int_t i, int_t j, int_t k, int_t l) override;

	// Java: func_31036_a — factory used by the piston base to create a
	// TileEntityPiston for the moving-block animation.
	static TileEntity *createMovingPistonTileEntity(int_t i, int_t j, int_t k, bool extending, bool isHead);
	static TileEntity *getTileEntity(int_t blockId, int_t metadata, int_t orientation, bool extending, bool isHead);

	AxisAlignedBB *getCollisionBoundingBoxFromPool(World *world, int_t i, int_t j, int_t k) override;
	void setBlockBoundsBasedOnState(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k) override;

	// Java: func_31035_a — collision box of the block currently being moved by a piston.
	AxisAlignedBB *getMovingBlockCollisionBox(World *world, int_t i, int_t j, int_t k, int_t blockID, float progress, int_t orientation);

protected:
	TileEntity *getBlockEntity() override;

private:
	// Java: func_31034_c — fetches the TileEntityPiston at (i,j,k) or null.
	TileEntityPiston *getPistonTileEntityAt(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k);
};
