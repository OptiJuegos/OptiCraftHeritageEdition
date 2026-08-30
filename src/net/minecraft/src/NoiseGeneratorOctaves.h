#pragma once

#include <memory>
#include <vector>

#include "NoiseBuffer.h"
#include "NoiseGenerator.h"
#include "java/Type.h"

class NoiseGeneratorPerlin;
class Random;

// net.minecraft.src.NoiseGeneratorOctaves
class NoiseGeneratorOctaves : public NoiseGenerator
{
public:
    NoiseGeneratorOctaves(Random &random, int_t octaves);
    ~NoiseGeneratorOctaves() override;

    TerrainNoiseBuffer &generateNoiseOctaves(TerrainNoiseBuffer &noise, double x, double y, double z,
                                             int_t sizeX, int_t sizeY, int_t sizeZ,
                                             double scaleX, double scaleY, double scaleZ);
#if PLATFORM_CONSOLE_LOW
    TerrainNoiseBuffer &generateNoiseOctavesFloat(TerrainNoiseBuffer &noise, terrain_coord_real_t x, terrain_coord_real_t y, terrain_coord_real_t z,
                                                  int_t sizeX, int_t sizeY, int_t sizeZ,
                                                  terrain_coord_real_t scaleX, terrain_coord_real_t scaleY, terrain_coord_real_t scaleZ);
#endif
    TerrainNoiseBuffer &getBiomeGenForCoords(TerrainNoiseBuffer &noise, int_t x, int_t z,
                                             int_t sizeX, int_t sizeZ,
                                             double scaleX, double scaleZ, double amplitudeScale);
#if PLATFORM_CONSOLE_LOW
    TerrainNoiseBuffer &getBiomeGenForCoordsFloat(TerrainNoiseBuffer &noise, int_t x, int_t z,
                                                  int_t sizeX, int_t sizeZ,
                                                  terrain_coord_real_t scaleX, terrain_coord_real_t scaleZ,
                                                  terrain_coord_real_t amplitudeScale);
#endif

private:
    std::vector<std::unique_ptr<NoiseGeneratorPerlin>> generatorCollection;
    int_t octaves;
};
