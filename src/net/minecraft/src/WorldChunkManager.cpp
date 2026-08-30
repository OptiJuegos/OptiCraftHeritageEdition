#include "WorldChunkManager.h"

#include "BiomeCache.h"
#include "BiomeGenBase.h"
#include "ChunkCoordIntPair.h"
#include "ChunkPosition.h"
#include "GenLayer.h"
#include "IntCache.h"
#include "World.h"
#include "WorldInfo.h"
#include "WorldType.h"
#include "java/Arithmetic.h"
#include "java/Random.h"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <stdexcept>

namespace
{
bool containsBiome(const std::vector<BiomeGenBase *> &biomes, BiomeGenBase *biome)
{
    return std::find(biomes.begin(), biomes.end(), biome) != biomes.end();
}

BiomeGenBase *biomeFromId(int_t id)
{
    if (id < 0 || id >= BiomeGenBase::BIOME_REGISTRY_SIZE || BiomeGenBase::biomeList[id] == nullptr)
        return BiomeGenBase::ocean;
    return BiomeGenBase::biomeList[id];
}

void requireLayerArea(const GenLayer::IntArray &ids, int_t count)
{
    if (ids.size() < static_cast<std::size_t>(count))
        throw std::length_error("WorldChunkManager: GenLayer returned undersized biome area");
}
}

WorldChunkManager::WorldChunkManager()
    : biomeCache125(std::make_unique<BiomeCache>(this))
{
    biomesToSpawnIn = {
        BiomeGenBase::forest,
        BiomeGenBase::plains,
        BiomeGenBase::taiga,
        BiomeGenBase::taigaHills,
        BiomeGenBase::forestHills,
        BiomeGenBase::jungle,
        BiomeGenBase::jungleHills
    };
}

WorldChunkManager::WorldChunkManager(long_t worldSeed, WorldType *worldType)
    : WorldChunkManager()
{
    if (worldType == nullptr)
        worldType = WorldType::DEFAULT;
    const std::array<GenLayer::Ptr, 3> layers = GenLayer::func_48425_a(worldSeed, worldType);
    genBiomes = layers[0];
    biomeIndexLayer = layers[1];
}

WorldChunkManager::WorldChunkManager(World *world)
    : WorldChunkManager(world->getRandomSeed(),
                        world->getWorldInfo() != nullptr ? world->getWorldInfo()->getTerrainType() : WorldType::DEFAULT)
{
}

WorldChunkManager::~WorldChunkManager() = default;


int_t WorldChunkManager::checkedBiomeAreaCount(int_t width, int_t height)
{
    if (width < 0 || height < 0)
        throw std::length_error("WorldChunkManager: negative biome area dimension");

    const std::uint64_t area = static_cast<std::uint64_t>(width) *
                               static_cast<std::uint64_t>(height);
    if (area > static_cast<std::uint64_t>(std::numeric_limits<int_t>::max()))
        throw std::length_error("WorldChunkManager: biome area exceeds Java array range");
    return static_cast<int_t>(area);
}

std::vector<BiomeGenBase *> &WorldChunkManager::getBiomesToSpawnIn()
{
    return biomesToSpawnIn;
}

BiomeGenBase *WorldChunkManager::getBiomeGenAtChunkCoord(ChunkCoordIntPair *chunkCoord)
{
    const int_t x = JavaArithmetic::intShl(chunkCoord->chunkXPos, 4);
    const int_t z = JavaArithmetic::intShl(chunkCoord->chunkZPos, 4);
    return getBiomeGenAt(x, z);
}

BiomeGenBase *WorldChunkManager::getBiomeGenAt(int_t x, int_t z)
{
    if (!biomeIndexLayer)
        return BiomeGenBase::plains;
    return biomeCache125->getBiomeGenAt(x, z);
}

double WorldChunkManager::getTemperature(int_t x, int_t z)
{
    return static_cast<double>(getBiomeGenAt(x, z)->getFloatTemperature());
}

BiomeNoiseBuffer &WorldChunkManager::getRainfall(BiomeNoiseBuffer &values,
                                                  int_t x, int_t z, int_t width, int_t height)
{
    IntCache::resetIntCache();
    const int_t count = checkedBiomeAreaCount(width, height);
    if (values.size() < static_cast<std::size_t>(count))
        values.resize(static_cast<std::size_t>(count));

    if (!biomeIndexLayer)
    {
        std::fill(values.begin(), values.begin() + count, static_cast<biome_noise_real_t>(0.5f));
        return values;
    }

    const GenLayer::IntArray &ids = biomeIndexLayer->getInts(x, z, width, height);
    requireLayerArea(ids, count);
    constexpr biome_noise_real_t inverseBiomeFixedPoint = static_cast<biome_noise_real_t>(1.0f / 65536.0f);
    for (int_t i = 0; i < count; ++i)
    {
        biome_noise_real_t rainfall = static_cast<biome_noise_real_t>(biomeFromId(ids[i])->getIntRainfall()) * inverseBiomeFixedPoint;
        if (rainfall > 1.0f)
            rainfall = 1.0f;
        values[i] = rainfall;
    }
    return values;
}

float WorldChunkManager::getTemperatureAtHeight(float temperatureValue, int_t)
{
    return temperatureValue;
}

BiomeNoiseBuffer &WorldChunkManager::getTemperatures(BiomeNoiseBuffer &values,
                                                      int_t x, int_t z, int_t width, int_t height)
{
    IntCache::resetIntCache();
    const int_t count = checkedBiomeAreaCount(width, height);
    if (values.size() < static_cast<std::size_t>(count))
        values.resize(static_cast<std::size_t>(count));

    if (!biomeIndexLayer)
    {
        std::fill(values.begin(), values.begin() + count, static_cast<biome_noise_real_t>(0.5f));
        return values;
    }

    const GenLayer::IntArray &ids = biomeIndexLayer->getInts(x, z, width, height);
    requireLayerArea(ids, count);
    constexpr biome_noise_real_t inverseBiomeFixedPoint = static_cast<biome_noise_real_t>(1.0f / 65536.0f);
    for (int_t i = 0; i < count; ++i)
    {
        biome_noise_real_t temp = static_cast<biome_noise_real_t>(biomeFromId(ids[i])->getIntTemperature()) * inverseBiomeFixedPoint;
        if (temp > 1.0f)
            temp = 1.0f;
        values[i] = temp;
    }
    return values;
}

std::vector<BiomeGenBase *> &WorldChunkManager::getBiomesForGeneration(std::vector<BiomeGenBase *> &values,
                                                                        int_t x, int_t z,
                                                                        int_t width, int_t height)
{
    IntCache::resetIntCache();
    const int_t count = checkedBiomeAreaCount(width, height);
    if (values.size() < static_cast<std::size_t>(count))
        values.resize(static_cast<std::size_t>(count));

    if (!genBiomes)
    {
        std::fill(values.begin(), values.begin() + count, BiomeGenBase::plains);
        return values;
    }

    const GenLayer::IntArray &ids = genBiomes->getInts(x, z, width, height);
    requireLayerArea(ids, count);
    for (int_t i = 0; i < count; ++i)
        values[i] = biomeFromId(ids[i]);
    return values;
}

std::vector<BiomeGenBase *> &WorldChunkManager::loadBlockGeneratorData(std::vector<BiomeGenBase *> &values,
                                                                        int_t x, int_t z,
                                                                        int_t width, int_t height)
{
    const int_t count = checkedBiomeAreaCount(width, height);
    if (values.size() < static_cast<std::size_t>(count))
        values.resize(static_cast<std::size_t>(count));
    getBiomeGenAt(values, x, z, width, height, true);
    return values;
}

std::vector<BiomeGenBase *> &WorldChunkManager::getBiomeGenAt(std::vector<BiomeGenBase *> &values,
                                                               int_t x, int_t z,
                                                               int_t width, int_t height,
                                                               bool useCache)
{
    IntCache::resetIntCache();
    const int_t count = checkedBiomeAreaCount(width, height);
    if (values.size() < static_cast<std::size_t>(count))
        values.resize(static_cast<std::size_t>(count));

    if (!biomeIndexLayer)
    {
        std::fill(values.begin(), values.begin() + count, BiomeGenBase::plains);
        return values;
    }

    if (useCache && width == 16 && height == 16 && (x & 15) == 0 && (z & 15) == 0)
    {
        const std::vector<BiomeGenBase *> &cached = biomeCache125->getCachedBiomes(x, z);
        if (cached.size() < static_cast<std::size_t>(count))
            throw std::length_error("WorldChunkManager: cached biome area is undersized");
        std::copy_n(cached.begin(), count, values.begin());
        return values;
    }

    const GenLayer::IntArray &ids = biomeIndexLayer->getInts(x, z, width, height);
    requireLayerArea(ids, count);
    for (int_t i = 0; i < count; ++i)
        values[i] = biomeFromId(ids[i]);
    return values;
}

bool WorldChunkManager::areBiomesViable(int_t x, int_t z, int_t radius,
                                         const std::vector<BiomeGenBase *> &allowedBiomes)
{
    IntCache::resetIntCache();
    const int_t minX = JavaArithmetic::intShr(JavaArithmetic::intSub(x, radius), 2);
    const int_t minZ = JavaArithmetic::intShr(JavaArithmetic::intSub(z, radius), 2);
    const int_t maxX = JavaArithmetic::intShr(JavaArithmetic::intAdd(x, radius), 2);
    const int_t maxZ = JavaArithmetic::intShr(JavaArithmetic::intAdd(z, radius), 2);
    const int_t width = JavaArithmetic::intAdd(JavaArithmetic::intSub(maxX, minX), 1);
    const int_t height = JavaArithmetic::intAdd(JavaArithmetic::intSub(maxZ, minZ), 1);

    const int_t count = checkedBiomeAreaCount(width, height);
    const GenLayer::IntArray &ids = genBiomes->getInts(minX, minZ, width, height);
    requireLayerArea(ids, count);
    for (int_t i = 0; i < count; ++i)
        if (!containsBiome(allowedBiomes, biomeFromId(ids[i])))
            return false;
    return true;
}

ChunkPosition *WorldChunkManager::findBiomePosition(int_t x, int_t z, int_t radius,
                                                     const std::vector<BiomeGenBase *> &allowedBiomes,
                                                     Random &random)
{
    IntCache::resetIntCache();
    const int_t minX = JavaArithmetic::intShr(JavaArithmetic::intSub(x, radius), 2);
    const int_t minZ = JavaArithmetic::intShr(JavaArithmetic::intSub(z, radius), 2);
    const int_t maxX = JavaArithmetic::intShr(JavaArithmetic::intAdd(x, radius), 2);
    const int_t maxZ = JavaArithmetic::intShr(JavaArithmetic::intAdd(z, radius), 2);
    const int_t width = JavaArithmetic::intAdd(JavaArithmetic::intSub(maxX, minX), 1);
    const int_t height = JavaArithmetic::intAdd(JavaArithmetic::intSub(maxZ, minZ), 1);
    const int_t count = checkedBiomeAreaCount(width, height);
    const GenLayer::IntArray &ids = genBiomes->getInts(minX, minZ, width, height);
    requireLayerArea(ids, count);

    ChunkPosition *position = nullptr;
    int_t matches = 0;
    for (int_t index = 0; index < count; ++index)
    {
        const int_t blockX = JavaArithmetic::intShl(JavaArithmetic::intAdd(minX, index % width), 2);
        const int_t blockZ = JavaArithmetic::intShl(JavaArithmetic::intAdd(minZ, index / width), 2);
        BiomeGenBase *biome = biomeFromId(ids[index]);
        if (containsBiome(allowedBiomes, biome) && (position == nullptr || random.nextInt(JavaArithmetic::intAdd(matches, 1)) == 0))
        {
            delete position;
            position = new ChunkPosition(blockX, 0, blockZ);
            matches = JavaArithmetic::intAdd(matches, 1);
        }
    }
    return position;
}

void WorldChunkManager::cleanupCache()
{
    biomeCache125->cleanupCache();
}

std::vector<BiomeGenBase *> &WorldChunkManager::getBiomeBlock(int_t x, int_t z, int_t width, int_t height)
{
    const int_t count = checkedBiomeAreaCount(width, height);
    if (biomeBuffer.size() < static_cast<std::size_t>(count))
        biomeBuffer.resize(static_cast<std::size_t>(count));
    getBiomeGenAt(biomeBuffer, x, z, width, height, true);

    getTemperatures(temperature, x, z, width, height);
    getRainfall(humidity, x, z, width, height);
    return biomeBuffer;
}
