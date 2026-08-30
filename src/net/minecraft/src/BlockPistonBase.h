#pragma once

#include "Block.h"
#include <vector>

class EntityLiving;
class EntityPlayer;

// net.minecraft.src.BlockPistonBase
class BlockPistonBase : public Block
{
public:
	BlockPistonBase(int_t i, int_t j, bool flag);

	int_t getPistonHeadTexture();
	int_t getPistonExtensionTexture();
	int_t getBlockTextureFromSideAndMetadata(int_t i, int_t j) override;
	int_t getRenderType() override;
	bool isOpaqueCube() override;
	bool blockActivated(World *world, int_t i, int_t j, int_t k, EntityPlayer *entityplayer) override;
	void onBlockPlacedBy(World *world, int_t i, int_t j, int_t k, EntityLiving *entityliving) override;
	void onNeighborBlockChange(World *world, int_t i, int_t j, int_t k, int_t l) override;
	void onBlockAdded(World *world, int_t i, int_t j, int_t k) override;
	void playBlock(World *world, int_t i, int_t j, int_t k, int_t l, int_t i1) override;
	void setBlockBoundsBasedOnState(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k) override;
	void setBlockBoundsForItemRender() override;
	void getCollidingBoundingBoxes(World *world, int_t i, int_t j, int_t k, AxisAlignedBB *axisalignedbb, std::vector<AxisAlignedBB *> &arraylist) override;
	AxisAlignedBB *getCollisionBoundingBoxFromPool(World *world, int_t i, int_t j, int_t k) override;
	bool renderAsNormalBlock() override;

	static int_t getPistonOrientation(int_t i);
	static bool isPowered(int_t i);
	static bool isExtended(int_t i);

private:
	void updatePistonState(World *world, int_t i, int_t j, int_t k);
	bool isIndirectlyPowered(World *world, int_t i, int_t j, int_t k, int_t l);
	static int_t getOrientationFromEntity(World *world, int_t i, int_t j, int_t k, EntityLiving *entityliving);
	static bool canPushBlock(int_t i, World *world, int_t j, int_t k, int_t l, bool flag);
	static bool canExtend(World *world, int_t i, int_t j, int_t k, int_t l);
	bool tryExtend(World *world, int_t i, int_t j, int_t k, int_t l);

	bool isSticky;
	bool isMoving;
};
