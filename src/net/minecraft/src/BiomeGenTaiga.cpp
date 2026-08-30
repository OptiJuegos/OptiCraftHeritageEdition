#include "BiomeGenTaiga.h"

#include <typeindex>

#include "BiomeDecorator.h"
#include "EntityWolf.h"
#include "WorldGenTaiga1.h"
#include "WorldGenTaiga2.h"

BiomeGenTaiga::BiomeGenTaiga()
{
    spawnableCreatureList.push_back(SpawnListEntry(std::type_index(typeid(EntityWolf)), 8, 4, 4));
    biomeDecorator->treesPerChunk = 10;
    biomeDecorator->grassPerChunk = 1;
}

WorldGenerator *BiomeGenTaiga::getRandomWorldGenForTrees(Random &random)
{
    if (random.nextInt(3) == 0)
        return new WorldGenTaiga1();
    return new WorldGenTaiga2(false);
}
