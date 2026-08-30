#pragma once

#include "Block.h"

class IBlockAccess;
class AxisAlignedBB;
class EntityPlayer;

// net.minecraft.src.BlockCake
class BlockCake : public Block
{
public:
	BlockCake(int_t i, int_t j);
	void setBlockBoundsBasedOnState(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k) override;
	void setBlockBoundsForItemRender() override;
	AxisAlignedBB *getCollisionBoundingBoxFromPool(World *world, int_t i, int_t j, int_t k) override;
	AxisAlignedBB *getSelectedBoundingBoxFromPool(World *world, int_t i, int_t j, int_t k) override;
	int_t getBlockTextureFromSideAndMetadata(int_t i, int_t j) override;
	int_t getBlockTextureFromSide(int_t i) override;
	bool renderAsNormalBlock() override;
	bool isOpaqueCube() override;
	bool blockActivated(World *world, int_t i, int_t j, int_t k, EntityPlayer *entityplayer) override;
	void onBlockClicked(World *world, int_t i, int_t j, int_t k, EntityPlayer *entityplayer) override;
	bool canPlaceBlockAt(World *world, int_t i, int_t j, int_t k) override;
	void onNeighborBlockChange(World *world, int_t i, int_t j, int_t k, int_t l) override;
	bool canBlockStay(World *world, int_t i, int_t j, int_t k) override;
	int_t quantityDropped(Random &random) override;
	int_t idDropped(int_t i, Random &random) override;

private:
	void eatCakeSlice(World *world, int_t i, int_t j, int_t k, EntityPlayer *entityplayer);
};
