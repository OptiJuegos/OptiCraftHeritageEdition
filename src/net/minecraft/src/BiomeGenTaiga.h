#pragma once

#include "BiomeGenBase.h"

class EntityWolf;
class WorldGenTaiga1;
class WorldGenTaiga2;

// net.minecraft.src.BiomeGenTaiga
class BiomeGenTaiga : public BiomeGenBase
{
public:
	BiomeGenTaiga();
	WorldGenerator *getRandomWorldGenForTrees(Random &random) override;
};
