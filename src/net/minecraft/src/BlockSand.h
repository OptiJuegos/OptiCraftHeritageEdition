#pragma once

#include "Block.h"

class World;
class EntityFallingSand;

// net.minecraft.src.BlockSand
class BlockSand : public Block
{
public:
	BlockSand(int_t i, int_t j);

	void onBlockAdded(World *world, int_t i, int_t j, int_t k) override;
	void onNeighborBlockChange(World *world, int_t i, int_t j, int_t k, int_t l) override;
	void updateTick(World *world, int_t i, int_t j, int_t k, Random &random) override;
	int_t tickRate() override;

	static bool canFallBelow(World *world, int_t i, int_t j, int_t k);

	static bool fallInstantly;

private:
	void tryToFall(World *world, int_t i, int_t j, int_t k);
};
