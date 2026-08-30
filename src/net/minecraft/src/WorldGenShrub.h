#pragma once

#include "WorldGenerator.h"

// net.minecraft.src.WorldGenShrub
class WorldGenShrub : public WorldGenerator
{
public:
    WorldGenShrub(int_t woodMetadata, int_t leavesMetadata);

    bool generate(World *world, Random &random, int_t x, int_t y, int_t z) override;

private:
    int_t leavesMetadata;
    int_t woodMetadata;
};
