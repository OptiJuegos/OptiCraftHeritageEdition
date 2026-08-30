#pragma once

#include "WorldChunkManager.h"

class BiomeGenBase;
class ChunkPosition;
class Random;

// net.minecraft.src.WorldChunkManagerHell
class WorldChunkManagerHell : public WorldChunkManager
{
public:
    WorldChunkManagerHell(BiomeGenBase *biome, float temperature, float rainfall);

    BiomeGenBase *getBiomeGenAt(int_t x, int_t z) override;
    double getTemperature(int_t x, int_t z) override;
    BiomeNoiseBuffer &getTemperatures(BiomeNoiseBuffer &values,
                                      int_t x, int_t z, int_t width, int_t height) override;
    BiomeNoiseBuffer &getRainfall(BiomeNoiseBuffer &values,
                                  int_t x, int_t z, int_t width, int_t height) override;
    std::vector<BiomeGenBase *> &getBiomesForGeneration(std::vector<BiomeGenBase *> &values,
                                                         int_t x, int_t z, int_t width, int_t height) override;
    std::vector<BiomeGenBase *> &loadBlockGeneratorData(std::vector<BiomeGenBase *> &values,
                                                         int_t x, int_t z, int_t width, int_t height) override;
    std::vector<BiomeGenBase *> &getBiomeGenAt(std::vector<BiomeGenBase *> &values,
                                                int_t x, int_t z, int_t width, int_t height,
                                                bool useCache) override;
    ChunkPosition *findBiomePosition(int_t x, int_t z, int_t radius,
                                      const std::vector<BiomeGenBase *> &allowedBiomes,
                                      Random &random) override;
    bool areBiomesViable(int_t x, int_t z, int_t radius,
                          const std::vector<BiomeGenBase *> &allowedBiomes) override;

private:
    BiomeGenBase *biomeGenerator;
    float hellTemperature;
    float rainfallValue;
};
