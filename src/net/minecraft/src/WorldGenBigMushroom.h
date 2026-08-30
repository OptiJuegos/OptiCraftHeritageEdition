#pragma once

#include "WorldGenerator.h"

// net.minecraft.src.WorldGenBigMushroom
class WorldGenBigMushroom : public WorldGenerator
{
public:
    WorldGenBigMushroom();
    explicit WorldGenBigMushroom(int_t mushroomType);

    bool generate(World *world, Random &random, int_t x, int_t y, int_t z) override;

private:
    int_t mushroomType;
};
