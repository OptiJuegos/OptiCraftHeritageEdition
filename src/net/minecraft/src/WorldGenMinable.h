#pragma once

#include "WorldGenerator.h"

// net.minecraft.src.WorldGenMinable
class WorldGenMinable : public WorldGenerator
{
public:
	WorldGenMinable(int_t i, int_t j);

	bool generate(World *world, Random &random, int_t i, int_t j, int_t k) override;

private:
	int_t minableBlockId;
	int_t numberOfBlocks;
};
