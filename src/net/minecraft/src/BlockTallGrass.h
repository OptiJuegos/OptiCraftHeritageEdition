#pragma once

#include "BlockFlower.h"

class IBlockAccess;

// net.minecraft.src.BlockTallGrass
class BlockTallGrass : public BlockFlower
{
public:
	BlockTallGrass(int_t i, int_t j);
	int_t getBlockTextureFromSideAndMetadata(int_t i, int_t j) override;
	int_t getBlockColor() override;
	int_t getRenderColor(int_t i) override;
	int_t colorMultiplier(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k) override;
	int_t idDropped(int_t i, Random &random) override;
	int_t quantityDroppedWithBonus(int_t fortune, Random &random) override;
	void harvestBlock(World *world, EntityPlayer *entityplayer, int_t i, int_t j, int_t k, int_t metadata) override;
};
