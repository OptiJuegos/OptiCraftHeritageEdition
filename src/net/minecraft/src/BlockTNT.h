#pragma once

#include "Block.h"

class EntityTNTPrimed;
class ItemStack;
class EntityPlayer;

// net.minecraft.src.BlockTNT
class BlockTNT : public Block
{
public:
	BlockTNT(int_t i, int_t j);
	int_t getBlockTextureFromSide(int_t i) override;
	void onBlockAdded(World *world, int_t i, int_t j, int_t k) override;
	void onNeighborBlockChange(World *world, int_t i, int_t j, int_t k, int_t l) override;
	int_t quantityDropped(Random &random) override;
	void onBlockDestroyedByExplosion(World *world, int_t i, int_t j, int_t k) override;
	void onBlockDestroyedByPlayer(World *world, int_t i, int_t j, int_t k, int_t l) override;
	void onBlockClicked(World *world, int_t i, int_t j, int_t k, EntityPlayer *entityplayer) override;
	bool blockActivated(World *world, int_t i, int_t j, int_t k, EntityPlayer *entityplayer) override;
	ItemStack *createStackedBlock(int_t metadata) override;
};
