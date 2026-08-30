#include "BiomeGenPlains.h"
#include "BiomeDecorator.h"
BiomeGenPlains::BiomeGenPlains()
{
    biomeDecorator->treesPerChunk = -999;
    biomeDecorator->flowersPerChunk = 4;
    biomeDecorator->grassPerChunk = 10;
}
