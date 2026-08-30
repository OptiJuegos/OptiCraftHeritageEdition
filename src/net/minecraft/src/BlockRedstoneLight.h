#pragma once

#include "Block.h"

// net.minecraft.src.BlockRedstoneLight
class BlockRedstoneLight : public Block
{
public:
	BlockRedstoneLight(int_t id, bool powered);

	void onBlockAdded(World *world, int_t x, int_t y, int_t z) override;
	void onNeighborBlockChange(World *world, int_t x, int_t y, int_t z, int_t neighborId) override;
	void updateTick(World *world, int_t x, int_t y, int_t z, Random &random) override;
	int_t idDropped(int_t metadata, Random &random, int_t fortune) override;

private:
	const bool powered;
};
