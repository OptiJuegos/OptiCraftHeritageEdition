#include "BiomeGenEnd.h"

#include <typeindex>

#include "BiomeEndDecorator.h"
#include "Block.h"
#include "EntityEnderman.h"

BiomeGenEnd::BiomeGenEnd()
{
    spawnableMonsterList.clear();
    spawnableCreatureList.clear();
    spawnableWaterCreatureList.clear();
    spawnableMonsterList.push_back(SpawnListEntry(std::type_index(typeid(EntityEnderman)), 10, 4, 4));
    topBlock = static_cast<byte_t>(Block::dirt->blockID);
    fillerBlock = static_cast<byte_t>(Block::dirt->blockID);

    delete biomeDecorator;
    biomeDecorator = createBiomeDecorator();
}

BiomeDecorator *BiomeGenEnd::createBiomeDecorator()
{
    return new BiomeEndDecorator(this);
}

int_t BiomeGenEnd::getSkyColorByTemp(float)
{
    return 0;
}
