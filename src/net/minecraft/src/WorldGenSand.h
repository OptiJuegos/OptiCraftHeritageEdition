#pragma once

#include "WorldGenerator.h"

// net.minecraft.src.WorldGenSand
class WorldGenSand : public WorldGenerator
{
public:
    WorldGenSand(int_t radius, int_t sandId);

    bool generate(World *world, Random &random, int_t x, int_t y, int_t z) override;

private:
    int_t sandID;
    int_t radius;
};
