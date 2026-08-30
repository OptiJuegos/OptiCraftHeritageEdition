#pragma once

#include "WorldGenerator.h"

// net.minecraft.src.WorldGenVines
class WorldGenVines : public WorldGenerator
{
public:
    bool generate(World *world, Random &random, int_t x, int_t y, int_t z) override;
};
