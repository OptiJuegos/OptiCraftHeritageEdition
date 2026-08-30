#pragma once
#include "NoiseBuffer.h"

#include <vector>

#include <array>
#include "NoiseGenerator.h"
#include "java/Random.h"
#include "java/Type.h"
#include "platform/PlatformConfig.h"

#if PLATFORM_CONSOLE_LOW
using perlin_real_t = float;
#else
using perlin_real_t = double;
#endif

// net.minecraft.src.NoiseGeneratorPerlin
class NoiseGeneratorPerlin : public NoiseGenerator
{
public:
	NoiseGeneratorPerlin();
	NoiseGeneratorPerlin(Random &random);

	perlin_real_t generateNoise(perlin_real_t d, perlin_real_t d1, perlin_real_t d2);
	// Java uses double throughout this math. Keep the low-CPU float shortcut
	// isolated so the default PC/Wii profiles preserve vanilla precision.
	perlin_real_t lerp(perlin_real_t d, perlin_real_t d1, perlin_real_t d2);
	perlin_real_t getBlockMetadata(int_t i, perlin_real_t d, perlin_real_t d1);
	perlin_real_t grad(int_t i, perlin_real_t d, perlin_real_t d1, perlin_real_t d2);
	perlin_real_t getDropItemId(perlin_real_t d, perlin_real_t d1);
	void getCanSpawnHere(TerrainNoiseBuffer &ad, perlin_real_t d, perlin_real_t d1, perlin_real_t d2,
	                int_t i, int_t j, int_t k,
	                perlin_real_t d3, perlin_real_t d4, perlin_real_t d5, perlin_real_t d6);

	perlin_real_t xCoord;
	perlin_real_t yCoord;
	perlin_real_t zCoord;

private:
	std::array<int_t, 512> permutations;
};
