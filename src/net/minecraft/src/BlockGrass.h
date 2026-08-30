#pragma once

#include "Block.h"

class IBlockAccess;

// net.minecraft.src.BlockGrass
class BlockGrass : public Block
{
public:
	BlockGrass(int_t i);
	int_t getBlockTextureFromSideAndMetadata(int_t side, int_t metadata) override;
	int_t getBlockTexture(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k, int_t l) override;
	int_t getBlockColor() override;
	int_t getRenderColor(int_t metadata) override;
	int_t colorMultiplier(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k) override;
	void updateTick(World *world, int_t i, int_t j, int_t k, Random &random) override;
	int_t idDropped(int_t i, Random &random) override;
	int_t idDropped(int_t i, Random &random, int_t fortune) override;
};
