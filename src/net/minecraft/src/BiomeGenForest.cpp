#include "BiomeGenForest.h"

#include <typeindex>

#include "BiomeDecorator.h"
#include "EntityWolf.h"
#include "WorldGenBigTree.h"
#include "WorldGenForest.h"
#include "WorldGenTrees.h"

BiomeGenForest::BiomeGenForest()
{
    spawnableCreatureList.push_back(SpawnListEntry(std::type_index(typeid(EntityWolf)), 5, 4, 4));
    biomeDecorator->treesPerChunk = 10;
    biomeDecorator->grassPerChunk = 2;
}

WorldGenerator *BiomeGenForest::getRandomWorldGenForTrees(Random &random)
{
    if (random.nextInt(5) == 0)
        return worldGenForest;
    if (random.nextInt(10) == 0)
        return worldGenBigTree;
    return worldGenTrees;
}
