#pragma once

#include "BiomeGenBase.h"

// net.minecraft.src.BiomeGenDesert
class BiomeGenDesert : public BiomeGenBase
{
public:
    BiomeGenDesert();
    void decorate(World *world, Random &random, int_t chunkX, int_t chunkZ) override;
};
