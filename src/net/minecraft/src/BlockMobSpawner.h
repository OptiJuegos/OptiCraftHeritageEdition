#pragma once

#include "BlockContainer.h"

class TileEntityMobSpawner;

// net.minecraft.src.BlockMobSpawner
class BlockMobSpawner : public BlockContainer
{
public:
	BlockMobSpawner(int_t i, int_t j);
	int_t idDropped(int_t i, Random &random) override;
	int_t quantityDropped(Random &random) override;
	bool isOpaqueCube() override;

protected:
	TileEntity *getBlockEntity() override;
};
