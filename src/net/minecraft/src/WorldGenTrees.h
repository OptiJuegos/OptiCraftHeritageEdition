#pragma once

#include "WorldGenerator.h"

// net.minecraft.src.WorldGenTrees
class WorldGenTrees : public WorldGenerator
{
public:
    WorldGenTrees();
    explicit WorldGenTrees(bool notify);
    WorldGenTrees(bool notify, int_t baseHeight, int_t woodMetadata,
                  int_t leavesMetadata, bool growVines);

    bool generate(World *world, Random &random, int_t x, int_t y, int_t z) override;

private:
    void growVinesDown(World *world, int_t x, int_t y, int_t z, int_t metadata);

    int_t baseHeight;
    bool growVines;
    int_t woodMetadata;
    int_t leavesMetadata;
};
