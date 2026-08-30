// Lightweight 2D heightmap terrain generator for weak consoles (PS2 / EE).
#include "java/Arithmetic.h"
//
// The vanilla Beta generator (ChunkProviderGenerate::generateTerrain) builds a
// 5x17x5 3D *density* field and runs it through several 8/16-octave Perlin
// stacks, all in `double`.  The EE has no hardware double FPU, so every one of
// those octave sweeps is software-emulated (~10-20x slower than float) and is
// the dominant cost of the multi-second stall when crossing a chunk border.
//
// This path replaces the 3D density field with a single per-column height value
// produced by an all-`float` fractal Perlin noise.  No doubles are touched here,
// and one column is sampled instead of a 425-point field x16 octaves -- roughly
// an order of magnitude cheaper.  The trade-off is purely visual: no overhangs,
// no floating islands and no noise-carved caves (caves are below the visible
// vertical window on console anyway).  Surface blocks, bedrock, ores, trees and
// decoration are untouched: we hand the same stone/water column shape to the
// stock replaceBlocksForBiome() and populate() afterwards.
//
// The whole translation unit compiles to nothing on PC builds.

#include "platform/PlatformTuning.h"

#if PLATFORM_USE_HEIGHTMAP_TERRAIN

#include "ChunkProviderGenerate.h"

#include "BiomeGenBase.h"
#include "LiteTerrainShape.h"
#include "World.h"
#include "WorldChunkManager.h"
#include "Block.h"
#include "java/Random.h"

#include <cstring>

namespace
{
	// --- All-float 2D Perlin noise -------------------------------------------
	// Self-contained so it never pulls in the shared double NoiseGeneratorPerlin.
	// Float keeps sub-block precision for world coords up to a few tens of
	// thousands of blocks, which is far past the console render/cache window.
	class LiteHeightNoise
	{
	public:
		long_t seed;

		explicit LiteHeightNoise(long_t worldSeed)
			: seed(worldSeed)
		{
			Random rand(worldSeed);
			for (int_t i = 0; i < 256; i++)
				perm[i] = (unsigned char)i;
			// Fisher-Yates with the project Random, same idiom as the vanilla
			// permutation table so the layout is deterministic per seed.
			for (int_t i = 0; i < 256; i++)
			{
				int_t j = rand.nextInt(256 - i) + i;
				unsigned char t = perm[i];
				perm[i] = perm[j];
				perm[j] = t;
			}
			for (int_t i = 0; i < 256; i++)
				perm[i + 256] = perm[i];

			// Random fractional offset so terrain is not mirror-symmetric about
			// the world origin.
			offX = (float)rand.nextInt(4096);
			offZ = (float)rand.nextInt(4096);
		}

		struct AxisSample
		{
			int_t cell;
			float fraction;
			float fadeValue;
		};

		void prepareChunk(int_t chunkBlockX, int_t chunkBlockZ)
		{
			for (int_t axis = 0; axis < 16; ++axis)
			{
				prepareAxis((float)JavaArithmetic::intAdd(chunkBlockX, axis) + offX, continentalX[axis], detailX[axis]);
				prepareAxis((float)JavaArithmetic::intAdd(chunkBlockZ, axis) + offZ, continentalZ[axis], detailZ[axis]);
			}
		}

		float surfaceYPrepared(int_t localX, int_t localZ, const LiteTerrain::BiomeShape &biome) const
		{
			const AxisSample *columnContinental = continentalX[localX];
			const AxisSample *rowContinental = continentalZ[localZ];
			const AxisSample *columnDetail = detailX[localX];
			const AxisSample *rowDetail = detailZ[localZ];
			const float continental = (perlin2Prepared(columnContinental[0], rowContinental[0])
				+ perlin2Prepared(columnContinental[1], rowContinental[1]) * 0.5f
				+ perlin2Prepared(columnContinental[2], rowContinental[2]) * 0.25f
				+ perlin2Prepared(columnContinental[3], rowContinental[3]) * 0.125f) / 1.875f;
			const float detail = (perlin2Prepared(columnDetail[0], rowDetail[0])
				+ perlin2Prepared(columnDetail[1], rowDetail[1]) * 0.5f) / 1.5f;

			const float base = (float)PLATFORM_HEIGHTMAP_BASE_HEIGHT;
			const float amp = (float)PLATFORM_HEIGHTMAP_AMPLITUDE;
			return LiteTerrain::surfaceHeight(base, amp, continental, detail, biome);
		}

	private:
		unsigned char perm[512];
		float offX;
		float offZ;
		AxisSample continentalX[16][4];
		AxisSample continentalZ[16][4];
		AxisSample detailX[16][2];
		AxisSample detailZ[16][2];

		static int_t ifloor(float v)
		{
			int_t i = (int_t)v;
			return v < (float)i ? i - 1 : i;
		}

		static float fade(float t) { return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f); }
		static float lerpf(float t, float a, float b) { return a + t * (b - a); }

		static float grad2(int_t hash, float x, float z)
		{
			switch (hash & 7)
			{
			case 0:  return  x + z;
			case 1:  return -x + z;
			case 2:  return  x - z;
			case 3:  return -x - z;
			case 4:  return  x;
			case 5:  return -x;
			case 6:  return  z;
			default: return -z;
			}
		}

		static AxisSample prepareAxisSample(float value)
		{
			const int_t cell = ifloor(value);
			const float fraction = value - (float)cell;
			return AxisSample{cell & 255, fraction, fade(fraction)};
		}

		static void prepareAxis(float coordinate, AxisSample (&continental)[4], AxisSample (&detail)[2])
		{
			float continentalCoordinate = coordinate * 0.0078125f;
			for (int_t octave = 0; octave < 4; ++octave)
			{
				continental[octave] = prepareAxisSample(continentalCoordinate);
				continentalCoordinate *= 2.0f;
			}

			float detailCoordinate = coordinate * 0.03125f + 100.0f;
			for (int_t octave = 0; octave < 2; ++octave)
			{
				detail[octave] = prepareAxisSample(detailCoordinate);
				detailCoordinate *= 2.0f;
			}
		}

		float perlin2Prepared(const AxisSample &x, const AxisSample &z) const
		{
			const int_t aa = perm[perm[x.cell] + z.cell];
			const int_t ab = perm[perm[x.cell] + z.cell + 1];
			const int_t ba = perm[perm[x.cell + 1] + z.cell];
			const int_t bb = perm[perm[x.cell + 1] + z.cell + 1];

			const float x1 = lerpf(x.fadeValue, grad2(aa, x.fraction, z.fraction),
				grad2(ba, x.fraction - 1.0f, z.fraction));
			const float x2 = lerpf(x.fadeValue, grad2(ab, x.fraction, z.fraction - 1.0f),
				grad2(bb, x.fraction - 1.0f, z.fraction - 1.0f));
			return lerpf(z.fadeValue, x1, x2);
		}
	};

	// One generator per world.  PS2 runs a single world at a time, so a lazily
	// (re)built instance keyed on the world seed is enough and avoids growing the
	// ChunkProviderGenerate layout / disturbing its RNG draw order.
	LiteHeightNoise *g_liteNoise = nullptr;

	LiteHeightNoise *liteNoiseFor(long_t seed)
	{
		if (g_liteNoise == nullptr || g_liteNoise->seed != seed)
		{
			delete g_liteNoise;
			g_liteNoise = new LiteHeightNoise(seed);
		}
		return g_liteNoise;
	}
}

void ChunkProviderGenerate::generateTerrainHeightmap(int_t i, int_t j, byte_t *abyte0)
{
	constexpr int_t biomeMargin = 2;
	constexpr int_t biomeStride = 16 + biomeMargin * 2;
	constexpr int_t chunkBiomeCount = 16 * 16;
	const int_t chunkBlockX = JavaArithmetic::intMul(i, 16);
	const int_t chunkBlockZ = JavaArithmetic::intMul(j, 16);
	const int_t biomeX = JavaArithmetic::intSub(chunkBlockX, biomeMargin);
	const int_t biomeZ = JavaArithmetic::intSub(chunkBlockZ, biomeMargin);

	worldObj->getWorldChunkManager()->loadBlockGeneratorData(
		liteBiomeHalo, biomeX, biomeZ, biomeStride, biomeStride);
	if (biomesForGeneration.size() < static_cast<std::size_t>(chunkBiomeCount))
		biomesForGeneration.resize(static_cast<std::size_t>(chunkBiomeCount));
	if (generatedTemperatures.size() < static_cast<std::size_t>(chunkBiomeCount))
		generatedTemperatures.resize(static_cast<std::size_t>(chunkBiomeCount));

	constexpr biome_noise_real_t inverseBiomeFixedPoint =
		static_cast<biome_noise_real_t>(1.0f / 65536.0f);
	for (int_t localX = 0; localX < 16; ++localX)
	{
		for (int_t localZ = 0; localZ < 16; ++localZ)
		{
			// Keep the existing PS2 X-major consumer layout. GenLayer itself is
			// Z-major, so changing this index order would rotate existing worlds.
			const std::size_t chunkIndex = static_cast<std::size_t>(localX * 16 + localZ);
			const std::size_t haloIndex = static_cast<std::size_t>(
				(localX + biomeMargin) * biomeStride + localZ + biomeMargin);
			BiomeGenBase *biome = liteBiomeHalo[haloIndex];
			biomesForGeneration[chunkIndex] = biome;

			biome_noise_real_t temperature =
				static_cast<biome_noise_real_t>(biome->getIntTemperature()) * inverseBiomeFixedPoint;
			if (temperature > static_cast<biome_noise_real_t>(1.0f))
				temperature = static_cast<biome_noise_real_t>(1.0f);
			generatedTemperatures[chunkIndex] = temperature;
		}
	}

	const int_t seaLevel   = PLATFORM_HEIGHTMAP_SEA_LEVEL;
	const byte_t stoneId   = (byte_t)Block::stone->blockID;
	const byte_t waterId   = (byte_t)Block::waterStill->blockID;
	const byte_t iceId     = (byte_t)Block::ice->blockID;

	LiteHeightNoise *noise = liteNoiseFor(worldObj->getRandomSeed());

	const std::size_t biomeSampleCount = static_cast<std::size_t>(biomeStride * biomeStride);
	if (liteBiomeSamples.size() < biomeSampleCount)
		liteBiomeSamples.resize(biomeSampleCount);
	for (std::size_t index = 0; index < biomeSampleCount; ++index)
	{
		BiomeGenBase *sample = liteBiomeHalo[index];
		const LiteTerrain::BiomeShape shape{sample->minHeight, sample->maxHeight};
		liteBiomeSamples[index] = LiteTerrain::makeBiomeBlendSample(shape);
	}

	noise->prepareChunk(chunkBlockX, chunkBlockZ);

	// abyte0 is a fresh 32768-zero buffer, so air (0) above the terrain needs no
	// writes.  Block index layout matches replaceBlocksForBiome():
	//   index = (l*16 + k)*128 + y , worldX = i*16 + l , worldZ = j*16 + k
	for (int_t l = 0; l < 16; l++)
	{
		for (int_t k = 0; k < 16; k++)
		{
			const int_t biomeX = l + 2;
			const int_t biomeZ = k + 2;
			const LiteTerrain::BiomeShape biome = LiteTerrain::blendBiomeShape(
				biomeX, biomeZ,
				[this, biomeStride](int_t x, int_t z) -> const LiteTerrain::BiomeBlendSample &
				{
					return liteBiomeSamples[static_cast<std::size_t>(x * biomeStride + z)];
				});

			int_t h = (int_t)noise->surfaceYPrepared(l, k, biome);
			if (h < 1)   h = 1;
			if (h > 120) h = 120;

			const int_t col = l * 16 + k;
			const int_t base = col * 128;
			liteColumnTop[col] = static_cast<byte_t>(h < seaLevel ? seaLevel - 1 : h);
			liteTerrainHeight[col] = static_cast<byte_t>(h);

			// Cold columns freeze the very top water block, mirroring the ice
			// rule the 3D generator applied at sea level - 1. Threshold is tunable
			// (PLATFORM_HEIGHTMAP_FREEZE_TEMP) so float biome noise does not sprinkle
			// ice onto temperate beaches.
			const bool cold = generatedTemperatures[col] < PLATFORM_HEIGHTMAP_FREEZE_TEMP;

			std::memset(abyte0 + base + 1, stoneId, static_cast<std::size_t>(h));

			if (h < seaLevel)
			{
				const int_t waterCount = seaLevel - h - 1;
				if (waterCount > 0)
					std::memset(abyte0 + base + h + 1, waterId, static_cast<std::size_t>(waterCount));
				if (cold && h < seaLevel - 1)
					abyte0[base + (seaLevel - 1)] = iceId;
			}
		}
	}
}


void ChunkProviderGenerate::replaceBlocksForBiomeHeightmap(byte_t *blocks, BiomeGenBase **biomes)
{
	constexpr int_t seaLevel = 63;
	const byte_t stoneId = static_cast<byte_t>(Block::stone->blockID);
	const byte_t bedrockId = static_cast<byte_t>(Block::bedrock->blockID);
	const byte_t iceId = static_cast<byte_t>(Block::ice->blockID);
	const byte_t waterId = static_cast<byte_t>(Block::waterStill->blockID);
	const byte_t sandId = static_cast<byte_t>(Block::sand->blockID);
	const byte_t sandstoneId = static_cast<byte_t>(Block::sandStone->blockID);

	for (int_t localX = 0; localX < 16; ++localX)
	{
		for (int_t localZ = 0; localZ < 16; ++localZ)
		{
			BiomeGenBase *biome = biomes[localZ + localX * 16];
			const float temperature = biome->getFloatTemperature();
			const int_t columnIndex = localZ * 16 + localX;
			const int_t base = columnIndex * 128;
			const int_t surfaceY = liteTerrainHeight[columnIndex] & 0xff;

			const terrain_noise_real_t depthRandom =
				static_cast<terrain_noise_real_t>(rand.nextDoubleFloat());
			const int_t depth = JavaArithmetic::floatToInt(
				stoneNoise[columnIndex] / static_cast<terrain_noise_real_t>(3.0f)
				+ static_cast<terrain_noise_real_t>(3.0f)
				+ depthRandom * static_cast<terrain_noise_real_t>(0.25f));

			byte_t topBlock = biome->topBlock;
			byte_t fillerBlock = biome->fillerBlock;
			int_t remainingDepth = depth;

			if (surfaceY >= 1 && blocks[base + surfaceY] == stoneId)
			{
				if (depth <= 0)
				{
					topBlock = 0;
					fillerBlock = stoneId;
				}
				else if (surfaceY >= seaLevel - 4 && surfaceY <= seaLevel + 1)
				{
					topBlock = biome->topBlock;
					fillerBlock = biome->fillerBlock;
				}

				if (surfaceY < seaLevel && topBlock == 0)
					topBlock = temperature < 0.15f ? iceId : waterId;

				blocks[base + surfaceY] = surfaceY >= seaLevel - 1 ? topBlock : fillerBlock;

				for (int_t y = surfaceY - 1; y >= 1 && remainingDepth > 0; --y)
				{
					--remainingDepth;
					blocks[base + y] = fillerBlock;
					if (remainingDepth == 0 && fillerBlock == sandId)
					{
						remainingDepth = rand.nextInt(4);
						fillerBlock = sandstoneId;
					}
				}
			}

			for (int_t y = 4; y >= 0; --y)
			{
				if (y <= rand.nextInt5())
					blocks[base + y] = bedrockId;
			}
		}
	}
}

#endif // PLATFORM_USE_HEIGHTMAP_TERRAIN
