#pragma once

#include "Block.h"

class EntityLiving;

// net.minecraft.src.BlockPumpkin
class BlockPumpkin : public Block
{
public:
	BlockPumpkin(int_t i, int_t j, bool flag);
	int_t getBlockTextureFromSideAndMetadata(int_t i, int_t j) override;
	int_t getBlockTextureFromSide(int_t i) override;
	void onBlockAdded(World *world, int_t i, int_t j, int_t k) override;
	bool canPlaceBlockAt(World *world, int_t i, int_t j, int_t k) override;
	void onBlockPlacedBy(World *world, int_t i, int_t j, int_t k, EntityLiving *entityliving) override;

private:
	bool blockType;
};
