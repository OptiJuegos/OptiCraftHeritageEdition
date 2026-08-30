#pragma once

#include "WorldGenerator.h"

// net.minecraft.src.WorldGenClay
class WorldGenClay : public WorldGenerator
{
public:
	WorldGenClay(int_t i);

	bool generate(World *world, Random &random, int_t i, int_t j, int_t k) override;

private:
	int_t clayBlockId;
	int_t numberOfBlocks;
};
