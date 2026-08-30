#pragma once

#include "WorldGenerator.h"

// net.minecraft.src.WorldGenTallGrass
class WorldGenTallGrass : public WorldGenerator
{
public:
	WorldGenTallGrass(int_t i, int_t j);

	bool generate(World *world, Random &random, int_t i, int_t j, int_t k) override;

private:
	int_t field_28060_a;
	int_t field_28059_b;
};
