#pragma once

#include "BiomeGenBase.h"

class EntityWolf;
class WorldGenForest;
class WorldGenBigTree;
class WorldGenTrees;

// net.minecraft.src.BiomeGenForest
class BiomeGenForest : public BiomeGenBase
{
public:
	BiomeGenForest();
	WorldGenerator *getRandomWorldGenForTrees(Random &random) override;
};
