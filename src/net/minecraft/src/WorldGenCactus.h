#pragma once

#include "WorldGenerator.h"

// net.minecraft.src.WorldGenCactus
class WorldGenCactus : public WorldGenerator
{
public:
	bool generate(World *world, Random &random, int_t i, int_t j, int_t k) override;
};
