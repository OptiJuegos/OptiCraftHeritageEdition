#include "BiomeGenDesert.h"

#include "BiomeDecorator.h"
#include "Block.h"
#include "World.h"
#include "WorldGenDesertWells.h"

BiomeGenDesert::BiomeGenDesert()
{
    spawnableCreatureList.clear();
    topBlock = (byte_t)Block::sand->blockID;
    fillerBlock = (byte_t)Block::sand->blockID;
    biomeDecorator->treesPerChunk = -999;
    biomeDecorator->deadBushPerChunk = 2;
    biomeDecorator->reedsPerChunk = 50;
    biomeDecorator->cactiPerChunk = 10;
}

void BiomeGenDesert::decorate(World *world, Random &random, int_t chunkX, int_t chunkZ)
{
    BiomeGenBase::decorate(world, random, chunkX, chunkZ);
    if (random.nextInt(1000) == 0)
    {
        const int_t x = chunkX + random.nextInt(16) + 8;
        const int_t z = chunkZ + random.nextInt(16) + 8;
        WorldGenDesertWells().generate(world, random, x, world->getHeightValue(x, z) + 1, z);
    }
}
