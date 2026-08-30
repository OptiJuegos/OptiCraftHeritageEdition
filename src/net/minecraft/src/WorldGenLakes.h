#pragma once

#include "WorldGenerator.h"

// net.minecraft.src.WorldGenLakes
class WorldGenLakes : public WorldGenerator
{
public:
	WorldGenLakes(int_t i);

	bool generate(World *world, Random &random, int_t i, int_t j, int_t k) override;

private:
	int_t field_15235_a;
};
