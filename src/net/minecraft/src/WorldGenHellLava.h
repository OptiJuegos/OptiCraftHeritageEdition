#pragma once

#include "WorldGenerator.h"

// net.minecraft.src.WorldGenHellLava
class WorldGenHellLava : public WorldGenerator
{
public:
	WorldGenHellLava(int_t i);

	bool generate(World *world, Random &random, int_t i, int_t j, int_t k) override;

private:
	int_t field_4158_a;
};
