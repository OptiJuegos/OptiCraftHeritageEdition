#pragma once

#include "WorldGenerator.h"

// net.minecraft.src.WorldGenSpikes
class WorldGenSpikes : public WorldGenerator
{
public:
    explicit WorldGenSpikes(int_t replaceId);
    bool generate(World *world, Random &random, int_t x, int_t y, int_t z) override;

private:
    int_t replaceID;
};
