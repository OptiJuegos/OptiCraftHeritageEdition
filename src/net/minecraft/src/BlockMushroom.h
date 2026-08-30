#pragma once

#include "BlockFlower.h"

class World;

// net.minecraft.src.BlockMushroom
class BlockMushroom : public BlockFlower
{
public:
	BlockMushroom(int_t i, int_t j);
	void updateTick(World *world, int_t i, int_t j, int_t k, Random &random) override;
	bool canPlaceBlockAt(World *world, int_t i, int_t j, int_t k) override;
	bool canBlockStay(World *world, int_t i, int_t j, int_t k) override;
	bool fertilizeMushroom(World *world, int_t i, int_t j, int_t k, Random &random);

protected:
	bool canThisPlantGrowOnThisBlockID(int_t i) override;
};
