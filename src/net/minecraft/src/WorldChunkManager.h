#pragma once

#include <memory>
#include <vector>

#include "NoiseBuffer.h"
#include "java/Type.h"

class BiomeCache;
class BiomeGenBase;
class ChunkCoordIntPair;
class ChunkPosition;
class GenLayer;
class Random;
class World;
class WorldType;

// net.minecraft.src.WorldChunkManager
class WorldChunkManager
{
public:
    WorldChunkManager();
    WorldChunkManager(long_t worldSeed, WorldType *worldType);
    explicit WorldChunkManager(World *world);
    virtual ~WorldChunkManager();

    virtual std::vector<BiomeGenBase *> &getBiomesToSpawnIn();
    virtual BiomeGenBase *getBiomeGenAtChunkCoord(ChunkCoordIntPair *chunkCoord);
    virtual BiomeGenBase *getBiomeGenAt(int_t x, int_t z);
    virtual double getTemperature(int_t x, int_t z);

    virtual BiomeNoiseBuffer &getRainfall(BiomeNoiseBuffer &values,
                                           int_t x, int_t z, int_t width, int_t height);
    virtual float getTemperatureAtHeight(float temperatureValue, int_t y);
    virtual BiomeNoiseBuffer &getTemperatures(BiomeNoiseBuffer &values,
                                               int_t x, int_t z, int_t width, int_t height);

    virtual std::vector<BiomeGenBase *> &getBiomesForGeneration(std::vector<BiomeGenBase *> &values,
                                                                 int_t x, int_t z, int_t width, int_t height);
    virtual std::vector<BiomeGenBase *> &loadBlockGeneratorData(std::vector<BiomeGenBase *> &values,
                                                                 int_t x, int_t z, int_t width, int_t height);
    virtual std::vector<BiomeGenBase *> &getBiomeGenAt(std::vector<BiomeGenBase *> &values,
                                                        int_t x, int_t z, int_t width, int_t height,
                                                        bool useCache);

    virtual bool areBiomesViable(int_t x, int_t z, int_t radius,
                                  const std::vector<BiomeGenBase *> &allowedBiomes);
    virtual ChunkPosition *findBiomePosition(int_t x, int_t z, int_t radius,
                                              const std::vector<BiomeGenBase *> &allowedBiomes,
                                              Random &random);
    virtual void cleanupCache();

    // Compatibility API used by Beta renderer color paths while those callers
    // are migrated to direct 1.2.5 biome temperature/rainfall lookups.
    virtual std::vector<BiomeGenBase *> &getBiomeBlock(int_t x, int_t z, int_t width, int_t height);

    BiomeNoiseBuffer temperature;
    BiomeNoiseBuffer humidity;

protected:
    static int_t checkedBiomeAreaCount(int_t width, int_t height);

    std::shared_ptr<GenLayer> genBiomes;
    std::shared_ptr<GenLayer> biomeIndexLayer;
    std::unique_ptr<BiomeCache> biomeCache125;
    std::vector<BiomeGenBase *> biomesToSpawnIn;
    std::vector<BiomeGenBase *> biomeBuffer;
};
