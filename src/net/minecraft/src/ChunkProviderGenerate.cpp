#include "ChunkProviderGenerate.h"
#include "java/Arithmetic.h"

#include <cmath>
#include <cstring>
#include <stdexcept>
#include <utility>

#include "World.h"
#include "Chunk.h"
#include "Block.h"
#include "BiomeGenBase.h"
#include "Material.h"
#include "MathHelper.h"
#include "MapGenCaves.h"
#include "MapGenRavine.h"
#include "MapGenMineshaft.h"
#include "MapGenVillage.h"
#include "MapGenStronghold.h"
#include "SpawnerAnimals.h"
#include "WorldChunkManager.h"
#include "WorldGenLakes.h"
#include "WorldGenDungeons.h"
#include "BlockSand.h"
#include "BlockDeadBush.h"
#include "BlockFlower.h"
#include "BlockTallGrass.h"
#include "platform/Log.h"
#include "platform/PlatformTuning.h"
#include "platform/WorldLoadTrace.h"
#include "platform/Diagnostics.h"

#if PLATFORM_PS2 && defined(PS2_RENDER_STATS)
#include "java/System.h"
#include "platform/Profiler.h"
#endif

ChunkProviderGenerate::ChunkProviderGenerate(World *world, long_t seed, bool mapFeaturesEnabledValue)
	: rand(seed)
	, field_912_k(rand, PLATFORM_TERRAIN_DENSITY_OCTAVES)
	, field_911_l(rand, PLATFORM_TERRAIN_DENSITY_OCTAVES)
	, field_910_m(rand, PLATFORM_TERRAIN_SELECT_OCTAVES)
	, field_909_n(rand, 4)
	, field_922_a(rand, 10)
	, field_921_b(rand, 16)
	, mobSpawnerNoise(rand, 8)
	, worldObj(world)
	, mapFeaturesEnabled(mapFeaturesEnabledValue)
	, biomesForGeneration()
{
	stoneNoise.resize(256);
	caveGenerator = new MapGenCaves();
	ravineGenerator = new MapGenRavine();
	mineshaftGenerator = new MapGenMineshaft();
	villageGenerator = new MapGenVillage(0);
	strongholdGenerator = new MapGenStronghold();
	std::memset(field_914_i, 0, sizeof(field_914_i));
#if PLATFORM_FAST_BIOME_BLEND
	biomeInverseHeightDenominators.resize(static_cast<std::size_t>(BiomeGenBase::BIOME_REGISTRY_SIZE), 0.0f);
	for (int_t biomeId = 0; biomeId < BiomeGenBase::BIOME_REGISTRY_SIZE; ++biomeId)
	{
		BiomeGenBase *biome = BiomeGenBase::biomeList[biomeId];
		if (biome == nullptr)
			continue;
		const float inverseHeightDenominator = 1.0f / (biome->minHeight + 2.0f);
		biomeInverseHeightDenominators[static_cast<std::size_t>(biomeId)] = inverseHeightDenominator;
	}
#endif
}

ChunkProviderGenerate::~ChunkProviderGenerate()
{
	delete caveGenerator;
	delete ravineGenerator;
	delete mineshaftGenerator;
	delete villageGenerator;
	delete strongholdGenerator;
}

void ChunkProviderGenerate::generateTerrain(int_t chunkX, int_t chunkZ, byte_t *blocks,
	BiomeGenBase **, const biome_noise_real_t *)
{
	constexpr int_t horizontalCells = 4;
	constexpr int_t verticalCells = 16;
	constexpr int_t seaLevel = 63;
	const int_t noiseWidth = horizontalCells + 1;
	constexpr int_t noiseHeight = 17;
	const int_t noiseDepth = horizontalCells + 1;

	worldObj->getWorldChunkManager()->getBiomesForGeneration(
		biomesForGeneration,
		JavaArithmetic::intSub(JavaArithmetic::intMul(chunkX, 4), 2),
		JavaArithmetic::intSub(JavaArithmetic::intMul(chunkZ, 4), 2),
		noiseWidth + 5,
		noiseDepth + 5);

	field_4180_q = generateNoiseField(field_4180_q,
		JavaArithmetic::intMul(chunkX, horizontalCells), 0,
		JavaArithmetic::intMul(chunkZ, horizontalCells),
		noiseWidth, noiseHeight, noiseDepth);

	for (int_t cellX = 0; cellX < horizontalCells; ++cellX)
	{
		for (int_t cellZ = 0; cellZ < horizontalCells; ++cellZ)
		{
			for (int_t cellY = 0; cellY < verticalCells; ++cellY)
			{
				constexpr terrain_noise_real_t verticalStep = static_cast<terrain_noise_real_t>(0.125f);
				terrain_noise_real_t density00 = field_4180_q[((cellX + 0) * noiseDepth + cellZ + 0) * noiseHeight + cellY + 0];
				terrain_noise_real_t density01 = field_4180_q[((cellX + 0) * noiseDepth + cellZ + 1) * noiseHeight + cellY + 0];
				terrain_noise_real_t density10 = field_4180_q[((cellX + 1) * noiseDepth + cellZ + 0) * noiseHeight + cellY + 0];
				terrain_noise_real_t density11 = field_4180_q[((cellX + 1) * noiseDepth + cellZ + 1) * noiseHeight + cellY + 0];
				const terrain_noise_real_t delta00 = (field_4180_q[((cellX + 0) * noiseDepth + cellZ + 0) * noiseHeight + cellY + 1] - density00) * verticalStep;
				const terrain_noise_real_t delta01 = (field_4180_q[((cellX + 0) * noiseDepth + cellZ + 1) * noiseHeight + cellY + 1] - density01) * verticalStep;
				const terrain_noise_real_t delta10 = (field_4180_q[((cellX + 1) * noiseDepth + cellZ + 0) * noiseHeight + cellY + 1] - density10) * verticalStep;
				const terrain_noise_real_t delta11 = (field_4180_q[((cellX + 1) * noiseDepth + cellZ + 1) * noiseHeight + cellY + 1] - density11) * verticalStep;

				for (int_t subY = 0; subY < 8; ++subY)
				{
					constexpr terrain_noise_real_t horizontalStep = static_cast<terrain_noise_real_t>(0.25f);
					terrain_noise_real_t left = density00;
					terrain_noise_real_t right = density01;
					const terrain_noise_real_t leftDelta = (density10 - density00) * horizontalStep;
					const terrain_noise_real_t rightDelta = (density11 - density01) * horizontalStep;

					for (int_t subX = 0; subX < 4; ++subX)
					{
						int_t index = ((subX + cellX * 4) << 11) | ((cellZ * 4) << 7) | (cellY * 8 + subY);
						constexpr int_t stride = 128;
						index -= stride;
						constexpr terrain_noise_real_t depthStep = static_cast<terrain_noise_real_t>(0.25f);
						const terrain_noise_real_t depthDelta = (right - left) * depthStep;
						terrain_noise_real_t density = left - depthDelta;

						for (int_t subZ = 0; subZ < 4; ++subZ)
						{
							density += depthDelta;
							index += stride;
							if (density > static_cast<terrain_noise_real_t>(0.0f))
								blocks[index] = static_cast<byte_t>(Block::stone->blockID);
							else if (cellY * 8 + subY < seaLevel)
								blocks[index] = static_cast<byte_t>(Block::waterStill->blockID);
							else
								blocks[index] = 0;
						}

						left += leftDelta;
						right += rightDelta;
					}

					density00 += delta00;
					density01 += delta01;
					density10 += delta10;
					density11 += delta11;
				}
			}
		}
	}
}

void ChunkProviderGenerate::replaceBlocksForBiome(int_t chunkX, int_t chunkZ, byte_t *blocks,
	BiomeGenBase **biomes)
{
	constexpr int_t seaLevel = 63;
#if PLATFORM_CONSOLE_LOW
	constexpr terrain_coord_real_t scale = static_cast<terrain_coord_real_t>(1.0f / 32.0f);
	stoneNoise = field_909_n.generateNoiseOctavesFloat(
		stoneNoise,
		static_cast<terrain_coord_real_t>(JavaArithmetic::intMul(chunkX, 16)),
		static_cast<terrain_coord_real_t>(JavaArithmetic::intMul(chunkZ, 16)),
		0.0f,
		16, 16, 1,
		scale * 2.0f, scale * 2.0f, scale * 2.0f);
#else
	constexpr double scale = 1.0 / 32.0;
	stoneNoise = field_909_n.generateNoiseOctaves(
		stoneNoise,
		static_cast<double>(JavaArithmetic::intMul(chunkX, 16)),
		static_cast<double>(JavaArithmetic::intMul(chunkZ, 16)),
		0.0,
		16, 16, 1,
		scale * 2.0, scale * 2.0, scale * 2.0);
#endif

#if PLATFORM_FAST_SURFACE_PASS && PLATFORM_USE_HEIGHTMAP_TERRAIN
	replaceBlocksForBiomeHeightmap(blocks, biomes);
	return;
#endif

	for (int_t localX = 0; localX < 16; ++localX)
	{
		for (int_t localZ = 0; localZ < 16; ++localZ)
		{
			BiomeGenBase *biome = biomes[localZ + localX * 16];
			const float temperature = biome->getFloatTemperature();
#if PLATFORM_CONSOLE_LOW
			const terrain_noise_real_t depthRandom = static_cast<terrain_noise_real_t>(rand.nextDoubleFloat());
			int_t depth = JavaArithmetic::floatToInt(
				stoneNoise[localX + localZ * 16] / static_cast<terrain_noise_real_t>(3.0f)
				+ static_cast<terrain_noise_real_t>(3.0f)
				+ depthRandom * static_cast<terrain_noise_real_t>(0.25f));
#else
			int_t depth = JavaArithmetic::doubleToInt(stoneNoise[localX + localZ * 16] / 3.0 + 3.0 + rand.nextDouble() * 0.25);
#endif
			int_t remainingDepth = -1;
			byte_t topBlock = biome->topBlock;
			byte_t fillerBlock = biome->fillerBlock;

			const int_t columnIndex = localZ * 16 + localX;
			for (int_t y = 127; y >= 0; --y)
			{
				const int_t index = columnIndex * 128 + y;
#if PLATFORM_CONSOLE_LOW
				if (y <= rand.nextInt5())
#else
				if (y <= rand.nextInt(5))
#endif
				{
					blocks[index] = static_cast<byte_t>(Block::bedrock->blockID);
					continue;
				}
#if PLATFORM_USE_HEIGHTMAP_TERRAIN
				if (y > (liteColumnTop[columnIndex] & 0xff))
					continue;
#endif

				const byte_t block = blocks[index];
				if (block == 0)
				{
					remainingDepth = -1;
					continue;
				}
				if (block != static_cast<byte_t>(Block::stone->blockID))
					continue;

				if (remainingDepth == -1)
				{
					if (depth <= 0)
					{
						topBlock = 0;
						fillerBlock = static_cast<byte_t>(Block::stone->blockID);
					}
					else if (y >= seaLevel - 4 && y <= seaLevel + 1)
					{
						topBlock = biome->topBlock;
						fillerBlock = biome->fillerBlock;
					}

					if (y < seaLevel && topBlock == 0)
					{
						topBlock = temperature < 0.15f
							? static_cast<byte_t>(Block::ice->blockID)
							: static_cast<byte_t>(Block::waterStill->blockID);
					}

					remainingDepth = depth;
					blocks[index] = y >= seaLevel - 1 ? topBlock : fillerBlock;
				}
				else if (remainingDepth > 0)
				{
					--remainingDepth;
					blocks[index] = fillerBlock;
					if (remainingDepth == 0 && fillerBlock == static_cast<byte_t>(Block::sand->blockID))
					{
						remainingDepth = rand.nextInt(4);
						fillerBlock = static_cast<byte_t>(Block::sandStone->blockID);
					}
				}
			}
		}
	}
}

Chunk *ChunkProviderGenerate::prepareChunk(int_t i, int_t j)
{
	return provideChunk(i, j);
}

Chunk *ChunkProviderGenerate::loadChunk(int_t i, int_t j)
{
    return provideChunk(i, j);
}

Chunk *ChunkProviderGenerate::provideChunk(int_t chunkX, int_t chunkZ)
{
	const ulong_t chunkSeedBits = static_cast<ulong_t>(static_cast<long_t>(chunkX)) * 341873128712ULL
	                            + static_cast<ulong_t>(static_cast<long_t>(chunkZ)) * 132897987541ULL;
	rand.setSeed(JavaArithmetic::longFromBits(chunkSeedBits));

	WORLD_LOAD_STAGE("provideChunk");
	if (WorldLoadTrace::active())
		MC_LOG_DEBUG("worldload", "provideChunk %d,%d\n", (int)chunkX, (int)chunkZ);
	WorldLoadTrace::step("terrain");
	std::vector<byte_t> blocks(32768, 0);
#if PLATFORM_USE_HEIGHTMAP_TERRAIN
	generateTerrainHeightmap(chunkX, chunkZ, blocks.data());
#else
	generateTerrain(chunkX, chunkZ, blocks.data(), nullptr, nullptr);
#endif

	WorldLoadTrace::step("biomes");
#if !PLATFORM_USE_HEIGHTMAP_TERRAIN
	worldObj->getWorldChunkManager()->loadBlockGeneratorData(
		biomesForGeneration,
		JavaArithmetic::intMul(chunkX, 16),
		JavaArithmetic::intMul(chunkZ, 16), 16, 16);
#endif
	replaceBlocksForBiome(chunkX, chunkZ, blocks.data(), biomesForGeneration.data());

#if !PLATFORM_SKIP_CAVE_GENERATION
	WorldLoadTrace::step("caves");
	caveGenerator->generate(this, worldObj, chunkX, chunkZ, blocks.data());
#endif
	WorldLoadTrace::step("ravines");
	ravineGenerator->generate(this, worldObj, chunkX, chunkZ, blocks.data());
	if (mapFeaturesEnabled)
	{
		WorldLoadTrace::step("mineshafts");
		mineshaftGenerator->generate(this, worldObj, chunkX, chunkZ, blocks.data());
		WorldLoadTrace::step("villages");
		villageGenerator->generate(this, worldObj, chunkX, chunkZ, blocks.data());
		WorldLoadTrace::step("strongholds");
		strongholdGenerator->generate(this, worldObj, chunkX, chunkZ, blocks.data());
	}

	WorldLoadTrace::step("newChunk");
	Chunk *chunk = new Chunk(worldObj, std::move(blocks), chunkX, chunkZ);
	WorldLoadTrace::step("biomeArray");
	std::vector<byte_t> &biomeArray = chunk->getBiomeArray();
	for (std::size_t i = 0; i < biomeArray.size() && i < biomesForGeneration.size(); ++i)
		biomeArray[i] = static_cast<byte_t>(biomesForGeneration[i]->biomeID);

	WorldLoadTrace::step("generateSkylightMap");
	if (WorldLoadTrace::active())
		platformHardwareCheckpoint("before generateSkylightMap");
	chunk->generateSkylightMap();
	if (WorldLoadTrace::active())
		platformHardwareCheckpoint("after generateSkylightMap");
	return chunk;
}

TerrainNoiseBuffer &ChunkProviderGenerate::generateNoiseField(TerrainNoiseBuffer &noise,
                                                               int_t x, int_t y, int_t z,
                                                               int_t sizeX, int_t sizeY, int_t sizeZ)
{
    return initializeNoiseField(noise, x, y, z, sizeX, sizeY, sizeZ);
}

TerrainNoiseBuffer &ChunkProviderGenerate::initializeNoiseField(TerrainNoiseBuffer &noise,
	int_t x, int_t y, int_t z, int_t width, int_t height, int_t depth)
{
	const std::size_t requiredSize = checkedNoiseVolumeSize(width, height, depth);
	if (noise.size() < requiredSize)
		noise.resize(requiredSize);

	if (biomeWeights.empty())
	{
		biomeWeights.resize(25);
		for (int_t offsetX = -2; offsetX <= 2; ++offsetX)
		{
			for (int_t offsetZ = -2; offsetZ <= 2; ++offsetZ)
			{
				const float distance = static_cast<float>(offsetX * offsetX + offsetZ * offsetZ) + 0.2f;
				biomeWeights[static_cast<std::size_t>(offsetX + 2 + (offsetZ + 2) * 5)] =
					10.0f / MathHelper::sqrt_float(distance);
			}
		}
	}

	constexpr terrain_noise_real_t horizontalScale = static_cast<terrain_noise_real_t>(684.412);
	constexpr terrain_noise_real_t verticalScale = static_cast<terrain_noise_real_t>(684.412);
#if PLATFORM_CONSOLE_LOW
	field_4182_g = field_922_a.getBiomeGenForCoordsFloat(field_4182_g, x, z, width, depth, 1.121f, 1.121f, 0.5f);
	field_4181_h = field_921_b.getBiomeGenForCoordsFloat(field_4181_h, x, z, width, depth, 200.0f, 200.0f, 0.5f);
#else
	field_4182_g = field_922_a.getBiomeGenForCoords(field_4182_g, x, z, width, depth, 1.121, 1.121, 0.5);
	field_4181_h = field_921_b.getBiomeGenForCoords(field_4181_h, x, z, width, depth, 200.0, 200.0, 0.5);
#endif
#if PLATFORM_CONSOLE_LOW
	const terrain_coord_real_t noiseX = static_cast<terrain_coord_real_t>(x);
	const terrain_coord_real_t noiseY = static_cast<terrain_coord_real_t>(y);
	const terrain_coord_real_t noiseZ = static_cast<terrain_coord_real_t>(z);
	field_4185_d = field_910_m.generateNoiseOctavesFloat(field_4185_d, noiseX, noiseY, noiseZ,
		width, height, depth, horizontalScale / 80.0f, verticalScale / 160.0f, horizontalScale / 80.0f);
	field_4184_e = field_912_k.generateNoiseOctavesFloat(field_4184_e, noiseX, noiseY, noiseZ,
		width, height, depth, horizontalScale, verticalScale, horizontalScale);
	field_4183_f = field_911_l.generateNoiseOctavesFloat(field_4183_f, noiseX, noiseY, noiseZ,
		width, height, depth, horizontalScale, verticalScale, horizontalScale);
#else
	field_4185_d = field_910_m.generateNoiseOctaves(field_4185_d, static_cast<double>(x), static_cast<double>(y), static_cast<double>(z),
		width, height, depth, horizontalScale / 80.0, verticalScale / 160.0, horizontalScale / 80.0);
	field_4184_e = field_912_k.generateNoiseOctaves(field_4184_e, static_cast<double>(x), static_cast<double>(y), static_cast<double>(z),
		width, height, depth, horizontalScale, verticalScale, horizontalScale);
	field_4183_f = field_911_l.generateNoiseOctaves(field_4183_f, static_cast<double>(x), static_cast<double>(y), static_cast<double>(z),
		width, height, depth, horizontalScale, verticalScale, horizontalScale);
#endif

	int_t noiseIndex = 0;
	int_t biomeNoiseIndex = 0;
	for (int_t localX = 0; localX < width; ++localX)
	{
		for (int_t localZ = 0; localZ < depth; ++localZ)
		{
			float averageMaxHeight = 0.0f;
			float averageMinHeight = 0.0f;
			float totalWeight = 0.0f;
			BiomeGenBase *centerBiome = biomesForGeneration[
				static_cast<std::size_t>(localX + 2 + (localZ + 2) * (width + 5))];

			for (int_t offsetX = -2; offsetX <= 2; ++offsetX)
			{
				for (int_t offsetZ = -2; offsetZ <= 2; ++offsetZ)
				{
					BiomeGenBase *neighbor = biomesForGeneration[
						static_cast<std::size_t>(localX + offsetX + 2 + (localZ + offsetZ + 2) * (width + 5))];
#if PLATFORM_FAST_BIOME_BLEND
					const float inverseHeightDenominator =
						biomeInverseHeightDenominators[static_cast<std::size_t>(neighbor->biomeID)];
					float weight = biomeWeights[static_cast<std::size_t>(offsetX + 2 + (offsetZ + 2) * 5)] *
						inverseHeightDenominator;
#else
					float weight = biomeWeights[static_cast<std::size_t>(offsetX + 2 + (offsetZ + 2) * 5)] /
						(neighbor->minHeight + 2.0f);
#endif
					if (neighbor->minHeight > centerBiome->minHeight)
						weight /= 2.0f;

					averageMaxHeight += neighbor->maxHeight * weight;
					averageMinHeight += neighbor->minHeight * weight;
					totalWeight += weight;
				}
			}

			averageMaxHeight /= totalWeight;
			averageMinHeight /= totalWeight;
			averageMaxHeight = averageMaxHeight * 0.9f + 0.1f;
			averageMinHeight = (averageMinHeight * 4.0f - 1.0f) / 8.0f;

			terrain_noise_real_t heightNoise = field_4181_h[static_cast<std::size_t>(biomeNoiseIndex)] / static_cast<terrain_noise_real_t>(8000.0);
			if (heightNoise < static_cast<terrain_noise_real_t>(0.0))
				heightNoise = -heightNoise * static_cast<terrain_noise_real_t>(0.3);
			heightNoise = heightNoise * static_cast<terrain_noise_real_t>(3.0) - static_cast<terrain_noise_real_t>(2.0);
			if (heightNoise < static_cast<terrain_noise_real_t>(0.0))
			{
				heightNoise /= static_cast<terrain_noise_real_t>(2.0);
				if (heightNoise < static_cast<terrain_noise_real_t>(-1.0))
					heightNoise = static_cast<terrain_noise_real_t>(-1.0);
				heightNoise /= static_cast<terrain_noise_real_t>(1.4);
				heightNoise /= static_cast<terrain_noise_real_t>(2.0);
			}
			else
			{
				if (heightNoise > static_cast<terrain_noise_real_t>(1.0))
					heightNoise = static_cast<terrain_noise_real_t>(1.0);
				heightNoise /= static_cast<terrain_noise_real_t>(8.0);
			}
			++biomeNoiseIndex;

			for (int_t localY = 0; localY < height; ++localY)
			{
				terrain_noise_real_t minHeight = static_cast<terrain_noise_real_t>(averageMinHeight);
				const terrain_noise_real_t maxHeight = static_cast<terrain_noise_real_t>(averageMaxHeight);
				minHeight += heightNoise * static_cast<terrain_noise_real_t>(0.2);
				minHeight = minHeight * static_cast<terrain_noise_real_t>(height) / static_cast<terrain_noise_real_t>(16.0);
				const terrain_noise_real_t center = static_cast<terrain_noise_real_t>(height) / static_cast<terrain_noise_real_t>(2.0) + minHeight * static_cast<terrain_noise_real_t>(4.0);
				terrain_noise_real_t verticalOffset = (static_cast<terrain_noise_real_t>(localY) - center) * static_cast<terrain_noise_real_t>(12.0) / maxHeight;
				if (verticalOffset < static_cast<terrain_noise_real_t>(0.0))
					verticalOffset *= static_cast<terrain_noise_real_t>(4.0);

				const terrain_noise_real_t low = field_4184_e[static_cast<std::size_t>(noiseIndex)] / static_cast<terrain_noise_real_t>(512.0);
				const terrain_noise_real_t high = field_4183_f[static_cast<std::size_t>(noiseIndex)] / static_cast<terrain_noise_real_t>(512.0);
				const terrain_noise_real_t blend = (field_4185_d[static_cast<std::size_t>(noiseIndex)] / static_cast<terrain_noise_real_t>(10.0) + static_cast<terrain_noise_real_t>(1.0)) / static_cast<terrain_noise_real_t>(2.0);
				terrain_noise_real_t density;
				if (blend < static_cast<terrain_noise_real_t>(0.0))
					density = low;
				else if (blend > static_cast<terrain_noise_real_t>(1.0))
					density = high;
				else
					density = low + (high - low) * blend;

				density -= verticalOffset;
				if (localY > height - 4)
				{
					const terrain_noise_real_t fade = static_cast<terrain_noise_real_t>(static_cast<float>(localY - (height - 4)) / 3.0f);
					density = density * (static_cast<terrain_noise_real_t>(1.0) - fade) + static_cast<terrain_noise_real_t>(-10.0) * fade;
				}

				noise[static_cast<std::size_t>(noiseIndex++)] = static_cast<terrain_noise_real_t>(density);
			}
		}
	}

	return noise;
}

bool ChunkProviderGenerate::chunkExists(int_t i, int_t j)
{
	return true;
}

void ChunkProviderGenerate::populate(IChunkProvider *ichunkprovider, int_t i, int_t j)
{
#if PLATFORM_INCREMENTAL_POPULATE
	while (!populateStep(ichunkprovider, i, j))
	{
	}
	return;
#else
	(void)ichunkprovider;
	BlockSand::fallInstantly = true;
	const int_t blockX = JavaArithmetic::intMul(i, 16);
	const int_t blockZ = JavaArithmetic::intMul(j, 16);
	BiomeGenBase *biome = worldObj->getBiomeGenForCoords(JavaArithmetic::intAdd(blockX, 16), JavaArithmetic::intAdd(blockZ, 16));

	rand.setSeed(worldObj->getRandomSeed());
	const long_t xMultiplier = (rand.nextLong() / 2LL) * 2LL + 1LL;
	const long_t zMultiplier = (rand.nextLong() / 2LL) * 2LL + 1LL;
	const ulong_t populateSeedBits =
		static_cast<ulong_t>(static_cast<long_t>(i)) * static_cast<ulong_t>(xMultiplier) +
		static_cast<ulong_t>(static_cast<long_t>(j)) * static_cast<ulong_t>(zMultiplier) ^
		static_cast<ulong_t>(worldObj->getRandomSeed());
	rand.setSeed(JavaArithmetic::longFromBits(populateSeedBits));

	bool villageGenerated = false;
	if (mapFeaturesEnabled)
	{
		mineshaftGenerator->generateStructuresInChunk(worldObj, rand, i, j);
		villageGenerated = villageGenerator->generateStructuresInChunk(worldObj, rand, i, j);
		strongholdGenerator->generateStructuresInChunk(worldObj, rand, i, j);
	}

	if (!villageGenerated && rand.nextInt(4) == 0)
	{
		const int_t x = JavaArithmetic::intAdd(JavaArithmetic::intAdd(blockX, rand.nextInt(16)), 8);
		const int_t y = rand.nextInt(128);
		const int_t z = JavaArithmetic::intAdd(JavaArithmetic::intAdd(blockZ, rand.nextInt(16)), 8);
		WorldGenLakes(Block::waterStill->blockID).generate(worldObj, rand, x, y, z);
	}

	if (!villageGenerated && rand.nextInt(8) == 0)
	{
		const int_t x = JavaArithmetic::intAdd(JavaArithmetic::intAdd(blockX, rand.nextInt(16)), 8);
		const int_t y = rand.nextInt(rand.nextInt(120) + 8);
		const int_t z = JavaArithmetic::intAdd(JavaArithmetic::intAdd(blockZ, rand.nextInt(16)), 8);
		if (y < 63 || rand.nextInt(10) == 0)
			WorldGenLakes(Block::lavaStill->blockID).generate(worldObj, rand, x, y, z);
	}

	for (int_t dungeon = 0; dungeon < PLATFORM_POPULATE_DUNGEONS; ++dungeon)
	{
		const int_t x = JavaArithmetic::intAdd(JavaArithmetic::intAdd(blockX, rand.nextInt(16)), 8);
		const int_t y = rand.nextInt(128);
		const int_t z = JavaArithmetic::intAdd(JavaArithmetic::intAdd(blockZ, rand.nextInt(16)), 8);
		WorldGenDungeons().generate(worldObj, rand, x, y, z);
	}

	if (biome != nullptr)
	{
		biome->decorate(worldObj, rand, blockX, blockZ);
		SpawnerAnimals::performWorldGenSpawning(
			worldObj, biome, JavaArithmetic::intAdd(blockX, 8), JavaArithmetic::intAdd(blockZ, 8), 16, 16, rand);
	}

	if (PLATFORM_POPULATE_SNOW_PASS)
	{
		const int_t precipitationX = JavaArithmetic::intAdd(blockX, 8);
		const int_t precipitationZ = JavaArithmetic::intAdd(blockZ, 8);
		for (int_t localX = 0; localX < 16; ++localX)
		{
			for (int_t localZ = 0; localZ < 16; ++localZ)
			{
				const int_t x = JavaArithmetic::intAdd(precipitationX, localX);
				const int_t z = JavaArithmetic::intAdd(precipitationZ, localZ);
				const int_t y = worldObj->getPrecipitationHeight(x, z);
				if (worldObj->isBlockHydratedDirectly(x, y - 1, z))
					worldObj->setBlockWithNotify(x, y - 1, z, Block::ice->blockID);
				if (worldObj->canSnowAt(x, y, z))
					worldObj->setBlockWithNotify(x, y, z, Block::snow->blockID);
			}
		}
	}

	BlockSand::fallInstantly = false;
#endif
}


bool ChunkProviderGenerate::saveChunks(bool flag, IProgressUpdate *iprogressupdate)
{
	return true;
}

bool ChunkProviderGenerate::unload100OldestChunks()
{
	return false;
}

bool ChunkProviderGenerate::canSave()
{
	return true;
}

jstring ChunkProviderGenerate::makeString()
{
	return "RandomLevelSource";
}

std::vector<SpawnListEntry> *ChunkProviderGenerate::getPossibleCreatures(const EnumCreatureType &type, int_t x, int_t y, int_t z)
{
	BiomeGenBase *biome = worldObj->getBiomeGenForCoords(x, z);
	return biome != nullptr ? biome->getSpawnableList(type) : nullptr;
}

ChunkPosition *ChunkProviderGenerate::findClosestStructure(World *world, const jstring &name, int_t x, int_t y, int_t z)
{
    if (name == "Stronghold" && strongholdGenerator != nullptr)
        return strongholdGenerator->getNearestInstance(world, x, y, z);
    return nullptr;
}
