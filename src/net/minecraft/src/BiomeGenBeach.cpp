#include "BiomeGenBeach.h"
#include "BiomeDecorator.h"
#include "Block.h"
BiomeGenBeach::BiomeGenBeach()
{
    spawnableCreatureList.clear();
    topBlock = (byte_t)Block::sand->blockID;
    fillerBlock = (byte_t)Block::sand->blockID;
    biomeDecorator->treesPerChunk = -999;
    biomeDecorator->deadBushPerChunk = 0;
    biomeDecorator->reedsPerChunk = 0;
    biomeDecorator->cactiPerChunk = 0;
}
