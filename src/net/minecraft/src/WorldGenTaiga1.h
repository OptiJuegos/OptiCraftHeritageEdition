#pragma once

#include "WorldGenerator.h"

// net.minecraft.src.WorldGenTaiga1
class WorldGenTaiga1 : public WorldGenerator
{
public:
	bool generate(World *world, Random &random, int_t i, int_t j, int_t k) override;
};
