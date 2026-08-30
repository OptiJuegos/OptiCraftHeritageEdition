#pragma once

#include "WorldGenerator.h"

// net.minecraft.src.WorldGenForest
class WorldGenForest : public WorldGenerator
{
public:
    WorldGenForest();
    explicit WorldGenForest(bool notify);

    bool generate(World *world, Random &random, int_t x, int_t y, int_t z) override;
};
