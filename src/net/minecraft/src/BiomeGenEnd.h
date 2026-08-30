#pragma once

#include "BiomeGenBase.h"

// net.minecraft.src.BiomeGenEnd
class BiomeGenEnd : public BiomeGenBase
{
public:
    BiomeGenEnd();
    int_t getSkyColorByTemp(float temperature) override;

protected:
    BiomeDecorator *createBiomeDecorator() override;
};
