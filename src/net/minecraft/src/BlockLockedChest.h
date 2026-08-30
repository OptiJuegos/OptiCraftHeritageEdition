#pragma once

#include "Block.h"

class IBlockAccess;

// net.minecraft.src.BlockLockedChest
class BlockLockedChest : public Block
{
public:
	BlockLockedChest(int_t i);
	int_t getBlockTexture(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k, int_t l) override;
	int_t getBlockTextureFromSide(int_t i) override;
	bool canPlaceBlockAt(World *world, int_t i, int_t j, int_t k) override;
	void updateTick(World *world, int_t i, int_t j, int_t k, Random &random) override;
};
