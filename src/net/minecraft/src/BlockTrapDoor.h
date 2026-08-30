#pragma once

#include "Block.h"

class IBlockAccess;
class AxisAlignedBB;
class EntityPlayer;
class Vec3D;
class MovingObjectPosition;

// net.minecraft.src.BlockTrapDoor
class BlockTrapDoor : public Block
{
public:
	bool getBlocksMovement(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k) override;
	BlockTrapDoor(int_t i, Material *material);
	bool isOpaqueCube() override;
	bool renderAsNormalBlock() override;
	int_t getRenderType() override;
	AxisAlignedBB *getSelectedBoundingBoxFromPool(World *world, int_t i, int_t j, int_t k) override;
	AxisAlignedBB *getCollisionBoundingBoxFromPool(World *world, int_t i, int_t j, int_t k) override;
	void setBlockBoundsBasedOnState(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k) override;
	void setBlockBoundsForItemRender() override;
	void setBlockBoundsForBlockRender(int_t i);
	void onBlockClicked(World *world, int_t i, int_t j, int_t k, EntityPlayer *entityplayer) override;
	bool blockActivated(World *world, int_t i, int_t j, int_t k, EntityPlayer *entityplayer) override;
	void onPoweredBlockChange(World *world, int_t i, int_t j, int_t k, bool flag);
	void onNeighborBlockChange(World *world, int_t i, int_t j, int_t k, int_t l) override;
	MovingObjectPosition *collisionRayTrace(World *world, int_t i, int_t j, int_t k, Vec3D *vec3d, Vec3D *vec3d1) override;
	void onBlockPlaced(World *world, int_t i, int_t j, int_t k, int_t l) override;
	bool canPlaceBlockOnSide(World *world, int_t i, int_t j, int_t k, int_t l) override;

	static bool isTrapdoorOpen(int_t i);

private:
	static bool isValidSupportBlock(int_t blockId);
};
