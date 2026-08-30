#pragma once

#include "Block.h"

class EntityPlayer;

// net.minecraft.src.BlockLog
class BlockLog : public Block
{
public:
	BlockLog(int_t i);
	int_t quantityDropped(Random &random) override;
	int_t idDropped(int_t i, Random &random) override;
	void harvestBlock(World *world, EntityPlayer *entityplayer, int_t i, int_t j, int_t k, int_t l) override;
	void onBlockRemoval(World *world, int_t i, int_t j, int_t k) override;
	int_t getBlockTextureFromSideAndMetadata(int_t i, int_t j) override;

protected:
	int_t damageDropped(int_t i) override;
};
