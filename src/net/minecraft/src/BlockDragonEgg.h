#pragma once

#include "Block.h"

// net.minecraft.src.BlockDragonEgg
class BlockDragonEgg : public Block
{
public:
	BlockDragonEgg(int_t id, int_t texture);

	void onBlockAdded(World *world, int_t x, int_t y, int_t z) override;
	void onNeighborBlockChange(World *world, int_t x, int_t y, int_t z, int_t neighborId) override;
	void updateTick(World *world, int_t x, int_t y, int_t z, Random &random) override;
	bool blockActivated(World *world, int_t x, int_t y, int_t z, EntityPlayer *entityplayer) override;
	void onBlockClicked(World *world, int_t x, int_t y, int_t z, EntityPlayer *entityplayer) override;
	int_t tickRate() override;
	bool canPlaceBlockAt(World *world, int_t x, int_t y, int_t z) override;
	bool isOpaqueCube() override;
	bool renderAsNormalBlock() override;
	int_t getRenderType() override;

private:
	void fallIfPossible(World *world, int_t x, int_t y, int_t z);
	void teleportNearby(World *world, int_t x, int_t y, int_t z);
};
