#pragma once

#include "WorldGenerator.h"

// net.minecraft.src.WorldGenSwamp
class WorldGenSwamp : public WorldGenerator
{
public:
    bool generate(World *world, Random &random, int_t x, int_t y, int_t z) override;

private:
    void generateVines(World *world, int_t x, int_t y, int_t z, int_t metadata);
};
