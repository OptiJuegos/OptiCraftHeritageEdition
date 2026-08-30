#include "BiomeGenMushroomIsland.h"
#include "BiomeDecorator.h"
#include "Block.h"
#include "BlockMycelium.h"
#include "EntityMooshroom.h"

#include <typeindex>
BiomeGenMushroomIsland::BiomeGenMushroomIsland()
{
    biomeDecorator->treesPerChunk = -100;
    biomeDecorator->flowersPerChunk = -100;
    biomeDecorator->grassPerChunk = -100;
    biomeDecorator->mushroomsPerChunk = 1;
    biomeDecorator->bigMushroomsPerChunk = 1;
    topBlock = (byte_t)Block::mycelium->blockID;
    spawnableMonsterList.clear();
    spawnableCreatureList.clear();
    spawnableWaterCreatureList.clear();
    spawnableCreatureList.push_back(SpawnListEntry(std::type_index(typeid(EntityMooshroom)), 8, 4, 8));
}
