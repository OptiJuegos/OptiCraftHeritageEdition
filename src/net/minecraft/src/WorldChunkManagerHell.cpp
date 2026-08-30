#include "WorldChunkManagerHell.h"

#include "BiomeGenBase.h"
#include "ChunkPosition.h"
#include "java/Arithmetic.h"
#include "java/Random.h"

#include <algorithm>

namespace
{
bool containsBiome(const std::vector<BiomeGenBase *> &biomes, BiomeGenBase *biome)
{
    return std::find(biomes.begin(), biomes.end(), biome) != biomes.end();
}
}

WorldChunkManagerHell::WorldChunkManagerHell(BiomeGenBase *biome, float temperatureValue, float rainfall)
    : WorldChunkManager(),
      biomeGenerator(biome),
      hellTemperature(temperatureValue),
      rainfallValue(rainfall)
{
}

BiomeGenBase *WorldChunkManagerHell::getBiomeGenAt(int_t, int_t)
{
    return biomeGenerator;
}

double WorldChunkManagerHell::getTemperature(int_t, int_t)
{
    return static_cast<double>(hellTemperature);
}

BiomeNoiseBuffer &WorldChunkManagerHell::getTemperatures(BiomeNoiseBuffer &values,
                                                          int_t, int_t, int_t width, int_t height)
{
    const int_t count = checkedBiomeAreaCount(width, height);
    if (values.size() < static_cast<std::size_t>(count))
        values.resize(static_cast<std::size_t>(count));
    std::fill(values.begin(), values.begin() + count, static_cast<biome_noise_real_t>(hellTemperature));
    return values;
}

BiomeNoiseBuffer &WorldChunkManagerHell::getRainfall(BiomeNoiseBuffer &values,
                                                      int_t, int_t, int_t width, int_t height)
{
    const int_t count = checkedBiomeAreaCount(width, height);
    if (values.size() < static_cast<std::size_t>(count))
        values.resize(static_cast<std::size_t>(count));
    std::fill(values.begin(), values.begin() + count, static_cast<biome_noise_real_t>(rainfallValue));
    return values;
}

std::vector<BiomeGenBase *> &WorldChunkManagerHell::getBiomesForGeneration(std::vector<BiomeGenBase *> &values,
                                                                            int_t, int_t, int_t width, int_t height)
{
    const int_t count = checkedBiomeAreaCount(width, height);
    if (values.size() < static_cast<std::size_t>(count))
        values.resize(static_cast<std::size_t>(count));
    std::fill(values.begin(), values.begin() + count, biomeGenerator);
    return values;
}

std::vector<BiomeGenBase *> &WorldChunkManagerHell::loadBlockGeneratorData(std::vector<BiomeGenBase *> &values,
                                                                            int_t, int_t, int_t width, int_t height)
{
    return getBiomesForGeneration(values, 0, 0, width, height);
}

std::vector<BiomeGenBase *> &WorldChunkManagerHell::getBiomeGenAt(std::vector<BiomeGenBase *> &values,
                                                                   int_t, int_t, int_t width, int_t height,
                                                                   bool)
{
    const int_t count = checkedBiomeAreaCount(width, height);
    if (values.size() < static_cast<std::size_t>(count))
        values.resize(static_cast<std::size_t>(count));
    std::fill(values.begin(), values.begin() + count, biomeGenerator);
    return values;
}

ChunkPosition *WorldChunkManagerHell::findBiomePosition(int_t x, int_t z, int_t radius,
                                                         const std::vector<BiomeGenBase *> &allowedBiomes,
                                                         Random &random)
{
    if (!containsBiome(allowedBiomes, biomeGenerator))
        return nullptr;

    const int_t diameter = JavaArithmetic::intAdd(JavaArithmetic::intMul(radius, 2), 1);
    const int_t resultX = JavaArithmetic::intAdd(JavaArithmetic::intSub(x, radius), random.nextInt(diameter));
    const int_t resultZ = JavaArithmetic::intAdd(JavaArithmetic::intSub(z, radius), random.nextInt(diameter));
    return new ChunkPosition(resultX, 0, resultZ);
}

bool WorldChunkManagerHell::areBiomesViable(int_t, int_t, int_t,
                                             const std::vector<BiomeGenBase *> &allowedBiomes)
{
    return containsBiome(allowedBiomes, biomeGenerator);
}
