#include "NoiseGeneratorOctaves.h"

#include <algorithm>
#include <cstddef>
#include <stdexcept>

#include "MathHelper.h"
#include "NoiseGeneratorPerlin.h"
#include "java/Random.h"

NoiseGeneratorOctaves::NoiseGeneratorOctaves(Random &random, int_t octaveCount)
    : octaves(octaveCount)
{
    if (octaveCount < 0)
        throw std::invalid_argument("NoiseGeneratorOctaves: negative octave count");
    generatorCollection.reserve(static_cast<std::size_t>(octaves));
    for (int_t i = 0; i < octaves; ++i)
        generatorCollection.emplace_back(new NoiseGeneratorPerlin(random));
}

NoiseGeneratorOctaves::~NoiseGeneratorOctaves() = default;

TerrainNoiseBuffer &NoiseGeneratorOctaves::generateNoiseOctaves(
    TerrainNoiseBuffer &noise, double x, double y, double z,
    int_t sizeX, int_t sizeY, int_t sizeZ,
    double scaleX, double scaleY, double scaleZ)
{
#if PLATFORM_CONSOLE_LOW
    return generateNoiseOctavesFloat(noise,
                                     static_cast<terrain_coord_real_t>(x),
                                     static_cast<terrain_coord_real_t>(y),
                                     static_cast<terrain_coord_real_t>(z),
                                     sizeX, sizeY, sizeZ,
                                     static_cast<terrain_coord_real_t>(scaleX),
                                     static_cast<terrain_coord_real_t>(scaleY),
                                     static_cast<terrain_coord_real_t>(scaleZ));
#else
    const std::size_t requiredSize = checkedNoiseVolumeSize(sizeX, sizeY, sizeZ);
    noise.resize(requiredSize);
    std::fill(noise.begin(), noise.end(), static_cast<terrain_noise_real_t>(0));
    double amplitude = 1.0;
    for (int_t i = 0; i < octaves; ++i)
    {
        double octaveX = x * amplitude * scaleX;
        const double octaveY = y * amplitude * scaleY;
        double octaveZ = z * amplitude * scaleZ;
        long_t wrappedX = MathHelper::floor_double_long(octaveX);
        long_t wrappedZ = MathHelper::floor_double_long(octaveZ);
        octaveX -= static_cast<double>(wrappedX);
        octaveZ -= static_cast<double>(wrappedZ);
        wrappedX %= 16777216LL;
        wrappedZ %= 16777216LL;
        octaveX += static_cast<double>(wrappedX);
        octaveZ += static_cast<double>(wrappedZ);
        generatorCollection[i]->getCanSpawnHere(
            noise,
            static_cast<perlin_real_t>(octaveX),
            static_cast<perlin_real_t>(octaveY),
            static_cast<perlin_real_t>(octaveZ),
            sizeX, sizeY, sizeZ,
            static_cast<perlin_real_t>(scaleX * amplitude),
            static_cast<perlin_real_t>(scaleY * amplitude),
            static_cast<perlin_real_t>(scaleZ * amplitude),
            static_cast<perlin_real_t>(amplitude));
        amplitude /= 2.0;
    }
#endif
    return noise;
}


#if PLATFORM_CONSOLE_LOW
TerrainNoiseBuffer &NoiseGeneratorOctaves::generateNoiseOctavesFloat(
    TerrainNoiseBuffer &noise, terrain_coord_real_t x, terrain_coord_real_t y, terrain_coord_real_t z,
    int_t sizeX, int_t sizeY, int_t sizeZ,
    terrain_coord_real_t scaleX, terrain_coord_real_t scaleY, terrain_coord_real_t scaleZ)
{
    const std::size_t requiredSize = checkedNoiseVolumeSize(sizeX, sizeY, sizeZ);
    noise.resize(requiredSize);
    std::fill(noise.begin(), noise.end(), static_cast<terrain_noise_real_t>(0));

    perlin_real_t amplitude = 1.0f;
    for (int_t i = 0; i < octaves; ++i)
    {
        perlin_real_t octaveX = static_cast<perlin_real_t>(x) * amplitude * static_cast<perlin_real_t>(scaleX);
        const perlin_real_t octaveY = static_cast<perlin_real_t>(y) * amplitude * static_cast<perlin_real_t>(scaleY);
        perlin_real_t octaveZ = static_cast<perlin_real_t>(z) * amplitude * static_cast<perlin_real_t>(scaleZ);

        long_t wrappedX = MathHelper::floor_double_long(static_cast<double>(octaveX));
        long_t wrappedZ = MathHelper::floor_double_long(static_cast<double>(octaveZ));
        octaveX -= static_cast<perlin_real_t>(wrappedX);
        octaveZ -= static_cast<perlin_real_t>(wrappedZ);
        wrappedX %= 16777216LL;
        wrappedZ %= 16777216LL;
        octaveX += static_cast<perlin_real_t>(wrappedX);
        octaveZ += static_cast<perlin_real_t>(wrappedZ);

        generatorCollection[i]->getCanSpawnHere(
            noise, octaveX, octaveY, octaveZ,
            sizeX, sizeY, sizeZ,
            static_cast<perlin_real_t>(scaleX) * amplitude,
            static_cast<perlin_real_t>(scaleY) * amplitude,
            static_cast<perlin_real_t>(scaleZ) * amplitude,
            amplitude);
        amplitude *= 0.5f;
    }
    return noise;
}
#endif

TerrainNoiseBuffer &NoiseGeneratorOctaves::getBiomeGenForCoords(
    TerrainNoiseBuffer &noise, int_t x, int_t z, int_t sizeX, int_t sizeZ,
    double scaleX, double scaleZ, double amplitudeScale)
{
    (void)amplitudeScale;
#if PLATFORM_CONSOLE_LOW
    return getBiomeGenForCoordsFloat(noise, x, z, sizeX, sizeZ,
                                     static_cast<terrain_coord_real_t>(scaleX),
                                     static_cast<terrain_coord_real_t>(scaleZ),
                                     static_cast<terrain_coord_real_t>(amplitudeScale));
#else
    return generateNoiseOctaves(noise, static_cast<double>(x), 10.0, static_cast<double>(z),
                                sizeX, 1, sizeZ, scaleX, 1.0, scaleZ);
#endif
}

#if PLATFORM_CONSOLE_LOW
TerrainNoiseBuffer &NoiseGeneratorOctaves::getBiomeGenForCoordsFloat(
    TerrainNoiseBuffer &noise, int_t x, int_t z, int_t sizeX, int_t sizeZ,
    terrain_coord_real_t scaleX, terrain_coord_real_t scaleZ, terrain_coord_real_t amplitudeScale)
{
    (void)amplitudeScale;
    return generateNoiseOctavesFloat(noise,
                                     static_cast<terrain_coord_real_t>(x),
                                     static_cast<terrain_coord_real_t>(10.0f),
                                     static_cast<terrain_coord_real_t>(z),
                                     sizeX, 1, sizeZ,
                                     scaleX, static_cast<terrain_coord_real_t>(1.0f), scaleZ);
}
#endif
