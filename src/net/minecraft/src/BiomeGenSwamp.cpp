#include "BiomeGenSwamp.h"

#include "BiomeDecorator.h"
#include "ColorizerFoliage.h"
#include "ColorizerGrass.h"
#include "WorldGenSwamp.h"
#include "platform/PlatformConfig.h"

BiomeGenSwamp::BiomeGenSwamp()
{
    biomeDecorator->treesPerChunk = 2;
    biomeDecorator->flowersPerChunk = -999;
    biomeDecorator->deadBushPerChunk = 1;
    biomeDecorator->mushroomsPerChunk = 8;
    biomeDecorator->reedsPerChunk = 10;
    biomeDecorator->clayPerChunk = 1;
    biomeDecorator->waterlilyPerChunk = 4;
    waterColorMultiplier = 14745518;
}

WorldGenerator *BiomeGenSwamp::getRandomWorldGenForTrees(Random &random)
{
    (void)random;
    return worldGenSwamp;
}

int_t BiomeGenSwamp::getBiomeGrassColor() const
{
#if PLATFORM_PS2
    return ((ColorizerGrass::getGrassColorByIndex(getCachedBiomeColorIndex()) & 16711422) + 5115470) / 2;
#else
    const double temp = (double)getFloatTemperature();
    const double rain = (double)getFloatRainfall();
    return ((ColorizerGrass::getGrassColor(temp, rain) & 16711422) + 5115470) / 2;
#endif
}

int_t BiomeGenSwamp::getBiomeFoliageColor() const
{
#if PLATFORM_PS2
    return ((ColorizerFoliage::getFoliageColorByIndex(getCachedBiomeColorIndex()) & 16711422) + 5115470) / 2;
#else
    const double temp = (double)getFloatTemperature();
    const double rain = (double)getFloatRainfall();
    return ((ColorizerFoliage::getFoliageColor(temp, rain) & 16711422) + 5115470) / 2;
#endif
}
