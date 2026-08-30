#include "BiomeGenHell.h"
#include <typeindex>
#include "EntityGhast.h"
#include "EntityMagmaCube.h"
#include "EntityPigZombie.h"

BiomeGenHell::BiomeGenHell()
{
    spawnableMonsterList.clear();
    spawnableCreatureList.clear();
    spawnableWaterCreatureList.clear();
    spawnableMonsterList.push_back(SpawnListEntry(std::type_index(typeid(EntityGhast)), 50, 4, 4));
    spawnableMonsterList.push_back(SpawnListEntry(std::type_index(typeid(EntityPigZombie)), 100, 4, 4));
    spawnableMonsterList.push_back(SpawnListEntry(std::type_index(typeid(EntityMagmaCube)), 1, 4, 4));
}
