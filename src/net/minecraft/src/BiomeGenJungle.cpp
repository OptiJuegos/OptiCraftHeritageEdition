#include "BiomeGenJungle.h"

#include "BiomeDecorator.h"
#include "Block.h"
#include "BlockTallGrass.h"
#include "EntityChicken.h"
#include "EntityOcelot.h"
#include "WorldGenBigTree.h"
#include "WorldGenHugeTrees.h"
#include "WorldGenShrub.h"
#include "WorldGenTallGrass.h"
#include "WorldGenTrees.h"
#include "WorldGenVines.h"

#include <typeindex>

BiomeGenJungle::BiomeGenJungle()
{
    biomeDecorator->treesPerChunk = 50;
    biomeDecorator->grassPerChunk = 25;
    biomeDecorator->flowersPerChunk = 4;

    spawnableMonsterList.push_back(SpawnListEntry(std::type_index(typeid(EntityOcelot)), 2, 1, 1));
    spawnableCreatureList.push_back(SpawnListEntry(std::type_index(typeid(EntityChicken)), 10, 4, 4));
}

WorldGenerator *BiomeGenJungle::getRandomWorldGenForTrees(Random &random)
{
    if (random.nextInt(10) == 0)
        return worldGenBigTree;
    if (random.nextInt(2) == 0)
        return new WorldGenShrub(3, 0);
    if (random.nextInt(3) == 0)
        return new WorldGenHugeTrees(false, 10 + random.nextInt(20), 3, 3);
    return new WorldGenTrees(false, 4 + random.nextInt(7), 3, 3, true);
}

WorldGenerator *BiomeGenJungle::func_48410_b(Random &random)
{
    return new WorldGenTallGrass(Block::tallGrass->blockID, random.nextInt(4) == 0 ? 2 : 1);
}

void BiomeGenJungle::decorate(World *world, Random &random, int_t chunkX, int_t chunkZ)
{
    BiomeGenBase::decorate(world, random, chunkX, chunkZ);

    WorldGenVines vines;
    for (int_t i = 0; i < 50; ++i)
    {
        const int_t x = chunkX + random.nextInt(16) + 8;
        const int_t z = chunkZ + random.nextInt(16) + 8;
        vines.generate(world, random, x, 64, z);
    }
}
