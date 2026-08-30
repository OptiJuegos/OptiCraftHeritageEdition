#include "BiomeEndDecorator.h"

#include "Block.h"
#include "EntityDragon.h"
#include "World.h"
#include "WorldGenSpikes.h"

BiomeEndDecorator::BiomeEndDecorator(BiomeGenBase *biome)
    : BiomeDecorator(biome), spikeGen(new WorldGenSpikes(Block::whiteStone->blockID))
{
}

void BiomeEndDecorator::decorate()
{
    generateOres();

    if (randomGenerator->nextInt(5) == 0)
    {
        const int_t x = chunk_X + randomGenerator->nextInt(16) + 8;
        const int_t z = chunk_Z + randomGenerator->nextInt(16) + 8;
        const int_t y = currentWorld->getTopSolidOrLiquidBlock(x, z);
        spikeGen->generate(currentWorld, *randomGenerator, x, y, z);
    }

    if (chunk_X == 0 && chunk_Z == 0)
    {
        EntityDragon *dragon = new EntityDragon(currentWorld);
        dragon->setLocationAndAngles(0.0, 128.0, 0.0,
                                     randomGenerator->nextFloat() * 360.0f, 0.0f);
        if (!currentWorld->spawnEntityInWorld(dragon))
            delete dragon;
    }
}
