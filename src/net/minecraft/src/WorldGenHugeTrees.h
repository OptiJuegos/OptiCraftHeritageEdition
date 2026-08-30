#pragma once

#include "WorldGenerator.h"

// net.minecraft.src.WorldGenHugeTrees
class WorldGenHugeTrees : public WorldGenerator
{
public:
    WorldGenHugeTrees(bool notify, int_t baseHeight, int_t woodMetadata, int_t leavesMetadata);

    bool generate(World *world, Random &random, int_t x, int_t y, int_t z) override;

private:
    void generateLeafNode(World *world, int_t x, int_t z, int_t y, int_t radius, Random &random);

    int_t baseHeight;
    int_t woodMetadata;
    int_t leavesMetadata;
};
