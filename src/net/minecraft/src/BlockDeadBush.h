#pragma once

#include "BlockFlower.h"

// net.minecraft.src.BlockDeadBush
class BlockDeadBush : public BlockFlower
{
public:
	BlockDeadBush(int_t i, int_t j);
	int_t getBlockTextureFromSideAndMetadata(int_t i, int_t j) override;
	int_t idDropped(int_t i, Random &random) override;
	void harvestBlock(World *world, EntityPlayer *entityplayer, int_t i, int_t j, int_t k, int_t l) override;

protected:
	bool canThisPlantGrowOnThisBlockID(int_t i) override;
};
