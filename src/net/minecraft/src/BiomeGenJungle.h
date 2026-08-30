#pragma once

#include "BiomeGenBase.h"

// net.minecraft.src.BiomeGenJungle
class BiomeGenJungle : public BiomeGenBase
{
public:
    BiomeGenJungle();

    WorldGenerator *getRandomWorldGenForTrees(Random &random) override;
    WorldGenerator *func_48410_b(Random &random) override;
    void decorate(World *world, Random &random, int_t chunkX, int_t chunkZ) override;
};
