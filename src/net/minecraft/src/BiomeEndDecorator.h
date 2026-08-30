#pragma once

#include <memory>

#include "BiomeDecorator.h"

class WorldGenerator;

// net.minecraft.src.BiomeEndDecorator
class BiomeEndDecorator : public BiomeDecorator
{
public:
    explicit BiomeEndDecorator(BiomeGenBase *biome);

protected:
    void decorate() override;

private:
    std::unique_ptr<WorldGenerator> spikeGen;
};
