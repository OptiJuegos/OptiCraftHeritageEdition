#pragma once

#include "WorldGenerator.h"

// net.minecraft.src.WorldGenLiquids
class WorldGenLiquids : public WorldGenerator
{
public:
	WorldGenLiquids(int_t i);

	bool generate(World *world, Random &random, int_t i, int_t j, int_t k) override;

private:
	int_t liquidBlockId;
};
