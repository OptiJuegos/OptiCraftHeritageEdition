#pragma once

#include "WorldGenerator.h"

// net.minecraft.src.WorldGenTaiga2
class WorldGenTaiga2 : public WorldGenerator
{
public:
	WorldGenTaiga2();
	explicit WorldGenTaiga2(bool notify);
	bool generate(World *world, Random &random, int_t i, int_t j, int_t k) override;
};
