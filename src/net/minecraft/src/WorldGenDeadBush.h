#pragma once

#include "WorldGenerator.h"

// net.minecraft.src.WorldGenDeadBush
class WorldGenDeadBush : public WorldGenerator
{
public:
	WorldGenDeadBush(int_t i);

	bool generate(World *world, Random &random, int_t i, int_t j, int_t k) override;

private:
	int_t field_28058_a;
};
