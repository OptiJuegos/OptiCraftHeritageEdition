#pragma once

#include "BiomeGenBase.h"

// net.minecraft.src.BiomeGenSwamp
class BiomeGenSwamp : public BiomeGenBase
{
public:
    BiomeGenSwamp();
    WorldGenerator *getRandomWorldGenForTrees(Random &random) override;
    int_t getBiomeGrassColor() const override;
    int_t getBiomeFoliageColor() const override;
};
