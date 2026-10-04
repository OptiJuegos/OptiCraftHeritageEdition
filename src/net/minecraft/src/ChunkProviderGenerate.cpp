#include "ChunkProviderGenerate.h"
#include "java/Arithmetic.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <stdexcept>
#include <utility>

#include "World.h"
#include "Chunk.h"
#include "Block.h"
#include "BiomeGenBase.h"
#include "BiomeDecorator.h"
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

namespace
{
	// Structure generation is a build profile on constrained consoles, not a world
	// property: WorldInfo still stores and saves the world's own MapFeatures flag,
	// so a save produced here generates structures normally on a desktop build.
	// See PLATFORM_GENERATE_MAP_FEATURES.
	bool platformMapFeaturesEnabled(bool worldSetting)
	{
		return worldSetting && PLATFORM_GENERATE_MAP_FEATURES != 0;
	}
}

ChunkProviderGenerate::ChunkProviderGenerate(World *world, long_t seed, bool mapFeaturesEnabledValue,
	bool isolatedBiomeSource)
	: rand(seed)
#if !PLATFORM_USE_HEIGHTMAP_TERRAIN
	, field_912_k(rand, PLATFORM_TERRAIN_DENSITY_OCTAVES, PLATFORM_TERRAIN_DENSITY_SKIP_FINE_OCTAVES)
	, field_911_l(rand, PLATFORM_TERRAIN_DENSITY_OCTAVES, PLATFORM_TERRAIN_DENSITY_SKIP_FINE_OCTAVES)
	, field_910_m(rand, PLATFORM_TERRAIN_SELECT_OCTAVES, PLATFORM_TERRAIN_SELECT_SKIP_FINE_OCTAVES)
	, field_909_n(rand, 4)
	, field_922_a(rand, 10)
	, field_921_b(rand, 16)
	, mobSpawnerNoise(rand, 8)
#endif
	, worldObj(world)
	, isolatedWorldChunkManager(isolatedBiomeSource && world != nullptr
		? std::make_unique<WorldChunkManager>(world) : nullptr)
	, mapFeaturesEnabled(platformMapFeaturesEnabled(mapFeaturesEnabledValue))
	, blocksScratch(32768, 0)
#if PLATFORM_CHUNK_LOCAL_DECORATION
	, metadataScratch(32768, 0)
#endif
	, biomesForGeneration()
{
	stoneNoise.resize(256);
	caveGenerator = new MapGenCaves();
	ravineGenerator = new MapGenRavine();
	if (isolatedWorldChunkManager != nullptr)
	{
		caveGenerator->setWorldChunkManagerOverride(isolatedWorldChunkManager.get());
		ravineGenerator->setWorldChunkManagerOverride(isolatedWorldChunkManager.get());
	}
	// Every call site guards on mapFeaturesEnabled (or, for findClosestStructure,
	// on the pointer itself), so leaving these null when the profile disables map
	// features also keeps their persistent structure maps out of the heap.
	if (mapFeaturesEnabled)
	{
		mineshaftGenerator = new MapGenMineshaft();
		villageGenerator = new MapGenVillage(0);
		strongholdGenerator = new MapGenStronghold();
	}
	else
	{
		mineshaftGenerator = nullptr;
		villageGenerator = nullptr;
		strongholdGenerator = nullptr;
	}
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
#if PLATFORM_INCREMENTAL_CHUNK_GENERATION
	cancelGenerationTask();
#endif
#if PLATFORM_INCREMENTAL_POPULATE
	if (populateTask.active)
		finishPopulateTask();
#endif
	delete caveGenerator;
	delete ravineGenerator;
	delete mineshaftGenerator;
	delete villageGenerator;
	delete strongholdGenerator;
}

WorldChunkManager *ChunkProviderGenerate::generationWorldChunkManager() const
{
	if (isolatedWorldChunkManager != nullptr)
		return isolatedWorldChunkManager.get();
	return worldObj != nullptr ? worldObj->getWorldChunkManager() : nullptr;
}

#if !PLATFORM_USE_HEIGHTMAP_TERRAIN
void ChunkProviderGenerate::generateTerrain(int_t chunkX, int_t chunkZ, byte_t *blocks,
	BiomeGenBase **, const biome_noise_real_t *)
{
	constexpr int_t horizontalCells = 4;
	constexpr int_t verticalCells = 16;
	constexpr int_t seaLevel = 63;
	const int_t noiseWidth = horizontalCells + 1;
	constexpr int_t noiseHeight = 17;
	const int_t noiseDepth = horizontalCells + 1;

	generationWorldChunkManager()->getBiomesForGeneration(
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

#endif // !PLATFORM_USE_HEIGHTMAP_TERRAIN

void ChunkProviderGenerate::replaceBlocksForBiome(int_t chunkX, int_t chunkZ, byte_t *blocks,
	BiomeGenBase **biomes)
{
	constexpr int_t seaLevel = 63;
#if PLATFORM_USE_HEIGHTMAP_TERRAIN
	// stoneNoise was already filled per column by generateTerrainHeightmap()
	// from the detail octaves it samples anyway; see ChunkProviderGenerateLite.cpp.
	(void)chunkX;
	(void)chunkZ;
#elif PLATFORM_FLOAT_TERRAIN_NOISE
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
#if PLATFORM_FLOAT_TERRAIN_NOISE
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
#if PLATFORM_FLOAT_TERRAIN_NOISE
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

void ChunkProviderGenerate::seedChunkGeneration(int_t chunkX, int_t chunkZ)
{
	const ulong_t chunkSeedBits = static_cast<ulong_t>(static_cast<long_t>(chunkX)) * 341873128712ULL
	                            + static_cast<ulong_t>(static_cast<long_t>(chunkZ)) * 132897987541ULL;
	rand.setSeed(JavaArithmetic::longFromBits(chunkSeedBits));
}

void ChunkProviderGenerate::generateBaseTerrain(int_t chunkX, int_t chunkZ, byte_t *blocks,
	byte_t biomeIds[256])
{
	seedChunkGeneration(chunkX, chunkZ);
	WorldLoadTrace::step("terrain");
	std::fill(blocks, blocks + 32768, static_cast<byte_t>(0));
#if PLATFORM_USE_HEIGHTMAP_TERRAIN
	generateTerrainHeightmap(chunkX, chunkZ, blocks);
#else
	generateTerrain(chunkX, chunkZ, blocks, nullptr, nullptr);
#endif

	WorldLoadTrace::step("biomes");
#if !PLATFORM_USE_HEIGHTMAP_TERRAIN
	generationWorldChunkManager()->loadBlockGeneratorData(
		biomesForGeneration,
		JavaArithmetic::intMul(chunkX, 16),
		JavaArithmetic::intMul(chunkZ, 16), 16, 16);
#endif
	replaceBlocksForBiome(chunkX, chunkZ, blocks, biomesForGeneration.data());

	for (std::size_t index = 0; index < 256 && index < biomesForGeneration.size(); ++index)
		biomeIds[index] = static_cast<byte_t>(biomesForGeneration[index]->biomeID);
}

void ChunkProviderGenerate::generateCaves(int_t chunkX, int_t chunkZ, byte_t *blocks)
{
#if !PLATFORM_SKIP_CAVE_GENERATION
	WorldLoadTrace::step("caves");
	caveGenerator->generate(this, worldObj, chunkX, chunkZ, blocks);
#else
	(void)chunkX;
	(void)chunkZ;
	(void)blocks;
#endif
}

void ChunkProviderGenerate::generateRavines(int_t chunkX, int_t chunkZ, byte_t *blocks)
{
#if !PLATFORM_SKIP_RAVINE_GENERATION
	WorldLoadTrace::step("ravines");
	ravineGenerator->generate(this, worldObj, chunkX, chunkZ, blocks);
#else
	(void)chunkX;
	(void)chunkZ;
	(void)blocks;
#endif
}

#if PLATFORM_INCREMENTAL_CHUNK_GENERATION
bool ChunkProviderGenerate::generateCavesStep(int_t chunkX, int_t chunkZ, byte_t *blocks, int_t &cursor)
{
#if !PLATFORM_SKIP_CAVE_GENERATION
	WorldLoadTrace::step("caves");
	return caveGenerator->generateRange(worldObj, chunkX, chunkZ, blocks, cursor,
		PLATFORM_GENERATION_SOURCE_COLUMNS_PER_STEP);
#else
	(void)chunkX;
	(void)chunkZ;
	(void)blocks;
	(void)cursor;
	return true;
#endif
}

bool ChunkProviderGenerate::generateRavinesStep(int_t chunkX, int_t chunkZ, byte_t *blocks, int_t &cursor)
{
#if !PLATFORM_SKIP_RAVINE_GENERATION
	WorldLoadTrace::step("ravines");
	return ravineGenerator->generateRange(worldObj, chunkX, chunkZ, blocks, cursor,
		PLATFORM_GENERATION_SOURCE_COLUMNS_PER_STEP);
#else
	(void)chunkX;
	(void)chunkZ;
	(void)blocks;
	(void)cursor;
	return true;
#endif
}

bool ChunkProviderGenerate::generateStructuresStep(int_t chunkX, int_t chunkZ, byte_t *blocks, int_t &cursor)
{
	MapGenStructure *structureMapGenerators[3];
	const int_t count = structureGenerators(structureMapGenerators);
	if (count == 0)
		return true;
	MapGenBase *generators[3];
	for (int_t index = 0; index < count; ++index)
		generators[index] = structureMapGenerators[index];
	WorldLoadTrace::step("structures");
	return MapGenBase::generateRangeShared(worldObj, chunkX, chunkZ, blocks, cursor,
		PLATFORM_STRUCTURE_SOURCE_COLUMNS_PER_STEP, generators, count);
}
#endif

int_t ChunkProviderGenerate::structureGenerators(MapGenStructure *out[3]) const
{
	if (!mapFeaturesEnabled)
		return 0;
	// Same order the three separate sweeps ran in.
	out[0] = mineshaftGenerator;
	out[1] = villageGenerator;
	out[2] = strongholdGenerator;
	return 3;
}

void ChunkProviderGenerate::generateStructures(int_t chunkX, int_t chunkZ, byte_t *blocks)
{
	MapGenStructure *structureMapGenerators[3];
	const int_t count = structureGenerators(structureMapGenerators);
	if (count == 0)
		return;
	MapGenBase *generators[3];
	for (int_t index = 0; index < count; ++index)
		generators[index] = structureMapGenerators[index];
	WorldLoadTrace::step("structures");
	int_t cursor = 0;
	MapGenBase::generateRangeShared(worldObj, chunkX, chunkZ, blocks, cursor, 0, generators, count);
}

void ChunkProviderGenerate::trimStructureStarts(int_t chunkX, int_t chunkZ)
{
#if PLATFORM_STRUCTURE_START_RETENTION_BLOCKS > 0
	if (!mapFeaturesEnabled)
		return;
	const int_t centerX = JavaArithmetic::intAdd(JavaArithmetic::intMul(chunkX, 16), 8);
	const int_t centerZ = JavaArithmetic::intAdd(JavaArithmetic::intMul(chunkZ, 16), 8);
	const int_t minX = JavaArithmetic::intSub(centerX, PLATFORM_STRUCTURE_START_RETENTION_BLOCKS);
	const int_t minZ = JavaArithmetic::intSub(centerZ, PLATFORM_STRUCTURE_START_RETENTION_BLOCKS);
	const int_t maxX = JavaArithmetic::intAdd(centerX, PLATFORM_STRUCTURE_START_RETENTION_BLOCKS);
	const int_t maxZ = JavaArithmetic::intAdd(centerZ, PLATFORM_STRUCTURE_START_RETENTION_BLOCKS);
	mineshaftGenerator->trimStructureStarts(minX, minZ, maxX, maxZ);
	villageGenerator->trimStructureStarts(minX, minZ, maxX, maxZ);
	strongholdGenerator->trimStructureStarts(minX, minZ, maxX, maxZ);
#else
	(void)chunkX;
	(void)chunkZ;
#endif
}

Chunk *ChunkProviderGenerate::buildGeneratedChunk(int_t chunkX, int_t chunkZ,
	const byte_t biomeIds[256], const byte_t *blocks, std::size_t blockCount,
	const byte_t *metadata)
{
	WorldLoadTrace::step("newChunk");
	Chunk *chunk = new Chunk(worldObj, blocks, blockCount, chunkX, chunkZ, metadata);
	WorldLoadTrace::step("biomeArray");
	std::vector<byte_t> &biomeArray = chunk->getBiomeArray();
	for (std::size_t index = 0; index < biomeArray.size() && index < 256; ++index)
		biomeArray[index] = biomeIds[index];
	return chunk;
}

void ChunkProviderGenerate::finishGeneratedChunkLighting(Chunk *chunk)
{
	if (chunk == nullptr)
		return;
	WorldLoadTrace::step("generateSkylightMap");
	if (WorldLoadTrace::active())
		platformHardwareCheckpoint("before generateSkylightMap");
	chunk->generateSkylightMap();
	if (WorldLoadTrace::active())
		platformHardwareCheckpoint("after generateSkylightMap");
}

bool ChunkProviderGenerate::generateAsyncChunkData(int_t chunkX, int_t chunkZ, std::vector<byte_t> &data)
{
	static constexpr std::size_t blockCount = 32768;
	static constexpr std::size_t biomeCount = 256;
	data.assign(blockCount + biomeCount, 0);

	byte_t *blocks = data.data();
	byte_t *biomeIds = data.data() + blockCount;
	generateBaseTerrain(chunkX, chunkZ, blocks, biomeIds);
	generateCaves(chunkX, chunkZ, blocks);
	generateRavines(chunkX, chunkZ, blocks);
	return true;
}

Chunk *ChunkProviderGenerate::finishAsyncChunkData(int_t chunkX, int_t chunkZ, std::vector<byte_t> &data)
{
	static constexpr std::size_t blockCount = 32768;
	static constexpr std::size_t biomeCount = 256;
	if (data.size() < blockCount + biomeCount)
		return nullptr;

	byte_t *blocks = data.data();
	const byte_t *biomeIds = data.data() + blockCount;
	generateStructures(chunkX, chunkZ, blocks);
	trimStructureStarts(chunkX, chunkZ);

#if PLATFORM_CHUNK_LOCAL_DECORATION
	decorateChunkLocal(chunkX, chunkZ, blocks, metadataScratch.data(), biomeIds);
	Chunk *chunk = buildGeneratedChunk(chunkX, chunkZ, biomeIds, blocks, blockCount,
		metadataScratch.data());
#else
	Chunk *chunk = buildGeneratedChunk(chunkX, chunkZ, biomeIds, blocks, blockCount);
#endif
	finishGeneratedChunkLighting(chunk);
	return chunk;
}

Chunk *ChunkProviderGenerate::provideChunk(int_t chunkX, int_t chunkZ)
{
	WORLD_LOAD_STAGE("provideChunk");
	if (WorldLoadTrace::active())
		MC_LOG_DEBUG("worldload", "provideChunk %d,%d\n", (int)chunkX, (int)chunkZ);

	byte_t biomeIds[256] = {};
	generateBaseTerrain(chunkX, chunkZ, blocksScratch.data(), biomeIds);
	generateCaves(chunkX, chunkZ, blocksScratch.data());
	generateRavines(chunkX, chunkZ, blocksScratch.data());
	generateStructures(chunkX, chunkZ, blocksScratch.data());
	trimStructureStarts(chunkX, chunkZ);
#if PLATFORM_CHUNK_LOCAL_DECORATION
	decorateChunkLocal(chunkX, chunkZ, blocksScratch.data(), metadataScratch.data(), biomeIds);
	Chunk *chunk = buildGeneratedChunk(
		chunkX, chunkZ, biomeIds, blocksScratch.data(), blocksScratch.size(), metadataScratch.data());
#else
	Chunk *chunk = buildGeneratedChunk(
		chunkX, chunkZ, biomeIds, blocksScratch.data(), blocksScratch.size());
#endif
	finishGeneratedChunkLighting(chunk);
	return chunk;
}


#if PLATFORM_INCREMENTAL_CHUNK_GENERATION
bool ChunkProviderGenerate::beginGenerationTask(int_t chunkX, int_t chunkZ)
{
	if (generationTask.active)
		return false;

	delete generationTask.chunk;
	generationTask.chunk = nullptr;
	if (generationTask.blocks.size() != 32768)
		generationTask.blocks.resize(32768);
#if PLATFORM_CHUNK_LOCAL_DECORATION
	if (generationTask.metadata.size() != 32768)
		generationTask.metadata.resize(32768);
#endif
	std::memset(generationTask.biomeIds, 0, sizeof(generationTask.biomeIds));
	generationTask.chunkX = chunkX;
	generationTask.chunkZ = chunkZ;
	generationTask.stage = GenerationStage::BaseTerrain;
	generationTask.sourceCursor = 0;
	generationTask.active = true;
	return true;
}

bool ChunkProviderGenerate::advanceGenerationTask()
{
	if (!generationTask.active || generationTask.stage == GenerationStage::Done)
		return false;

	WORLD_LOAD_STAGE("provideChunkIncremental");
	if (WorldLoadTrace::active())
	{
		MC_LOG_DEBUG("worldload", "provideChunkIncremental %d,%d stage=%d\n",
			(int)generationTask.chunkX, (int)generationTask.chunkZ,
			(int)generationTask.stage);
	}

	byte_t *blocks = generationTask.blocks.data();
	switch (generationTask.stage)
	{
	case GenerationStage::BaseTerrain:
		generateBaseTerrain(generationTask.chunkX, generationTask.chunkZ, blocks, generationTask.biomeIds);
		generationTask.stage = GenerationStage::Caves;
		break;
	case GenerationStage::Caves:
		// Sliced: stay on this stage until the source sweep reports completion, so
		// the caller's time budget can stop between slices.
		if (generateCavesStep(generationTask.chunkX, generationTask.chunkZ, blocks,
				generationTask.sourceCursor))
		{
			generationTask.sourceCursor = 0;
			generationTask.stage = GenerationStage::Ravines;
		}
		break;
	case GenerationStage::Ravines:
		if (generateRavinesStep(generationTask.chunkX, generationTask.chunkZ, blocks,
				generationTask.sourceCursor))
		{
			generationTask.sourceCursor = 0;
#if PLATFORM_CHUNK_LOCAL_DECORATION
			generationTask.stage = mapFeaturesEnabled ? GenerationStage::Structures : GenerationStage::Decorate;
#else
			generationTask.stage = mapFeaturesEnabled ? GenerationStage::Structures : GenerationStage::BuildChunk;
#endif
		}
		break;
	case GenerationStage::Structures:
		if (generateStructuresStep(generationTask.chunkX, generationTask.chunkZ, blocks,
			generationTask.sourceCursor))
		{
			generationTask.sourceCursor = 0;
#if PLATFORM_CHUNK_LOCAL_DECORATION
			generationTask.stage = GenerationStage::Decorate;
#else
			generationTask.stage = GenerationStage::BuildChunk;
#endif
		}
		break;
#if PLATFORM_CHUNK_LOCAL_DECORATION
	case GenerationStage::Decorate:
		decorateChunkLocal(generationTask.chunkX, generationTask.chunkZ, blocks,
			generationTask.metadata.data(), generationTask.biomeIds);
		generationTask.stage = GenerationStage::BuildChunk;
		break;
#endif
	case GenerationStage::BuildChunk:
		generationTask.chunk = buildGeneratedChunk(
			generationTask.chunkX, generationTask.chunkZ, generationTask.biomeIds,
			blocks, generationTask.blocks.size()
#if PLATFORM_CHUNK_LOCAL_DECORATION
			, generationTask.metadata.data()
#endif
			);
		generationTask.stage = GenerationStage::Skylight;
		break;
	case GenerationStage::Skylight:
		finishGeneratedChunkLighting(generationTask.chunk);
		generationTask.stage = GenerationStage::Done;
		break;
	case GenerationStage::Done:
		return false;
	}
	return true;
}

void ChunkProviderGenerate::cancelGenerationTask()
{
	delete generationTask.chunk;
	generationTask.chunk = nullptr;
	generationTask.active = false;
	generationTask.chunkX = 0;
	generationTask.chunkZ = 0;
	generationTask.stage = GenerationStage::BaseTerrain;
	generationTask.sourceCursor = 0;
}

bool ChunkProviderGenerate::hasGenerationTask() const
{
	return generationTask.active;
}

int_t ChunkProviderGenerate::generationTaskX() const
{
	return generationTask.chunkX;
}

int_t ChunkProviderGenerate::generationTaskZ() const
{
	return generationTask.chunkZ;
}

Chunk *ChunkProviderGenerate::takeGeneratedChunk()
{
	if (!generationTask.active || generationTask.stage != GenerationStage::Done)
		return nullptr;

	Chunk *chunk = generationTask.chunk;
	generationTask.chunk = nullptr;
	generationTask.active = false;
	generationTask.chunkX = 0;
	generationTask.chunkZ = 0;
	generationTask.stage = GenerationStage::BaseTerrain;
	generationTask.sourceCursor = 0;
	return chunk;
}
#endif

#if !PLATFORM_USE_HEIGHTMAP_TERRAIN
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
#if PLATFORM_FLOAT_TERRAIN_NOISE
	field_4182_g = field_922_a.getBiomeGenForCoordsFloat(field_4182_g, x, z, width, depth, 1.121f, 1.121f, 0.5f);
	field_4181_h = field_921_b.getBiomeGenForCoordsFloat(field_4181_h, x, z, width, depth, 200.0f, 200.0f, 0.5f);
#else
	field_4182_g = field_922_a.getBiomeGenForCoords(field_4182_g, x, z, width, depth, 1.121, 1.121, 0.5);
	field_4181_h = field_921_b.getBiomeGenForCoords(field_4181_h, x, z, width, depth, 200.0, 200.0, 0.5);
#endif
#if PLATFORM_FLOAT_TERRAIN_NOISE
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
			if (worldObj != nullptr && worldObj->isLimitedWorld())
			{
				const double blockX = static_cast<double>((x + localX) * 4);
				const double blockZ = static_cast<double>((z + localZ) * 4);
				const double dist = std::sqrt(blockX * blockX + blockZ * blockZ);
				if (dist < 88.0)
				{
					if (averageMaxHeight > 0.2f)
						averageMaxHeight *= 1.25f;
					averageMinHeight += 0.25f;
				}
				else if (dist < 118.0)
				{
					const double t = (dist - 88.0) / 30.0;
					averageMinHeight = (averageMinHeight + 0.25f) * (1.0 - t) + (-1.0f) * t;
				}
				else
				{
					averageMinHeight = -1.0f;
				}
			}

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

			const terrain_noise_real_t heightVarianceScale = (worldObj != nullptr && worldObj->isLimitedWorld())
				? static_cast<terrain_noise_real_t>(0.32)
				: static_cast<terrain_noise_real_t>(0.2);

			for (int_t localY = 0; localY < height; ++localY)
			{
				terrain_noise_real_t minHeight = static_cast<terrain_noise_real_t>(averageMinHeight);
				const terrain_noise_real_t maxHeight = static_cast<terrain_noise_real_t>(averageMaxHeight);
				minHeight += heightNoise * heightVarianceScale;
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
#endif // !PLATFORM_USE_HEIGHTMAP_TERRAIN

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

#if PLATFORM_POPULATE_LAKES
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
#else
	// PLATFORM_POPULATE_LAKES stays authoritative for limited worlds too: the
	// MCPE profile that arrived from main rolled a 1-in-12 water lake here
	// even on the bounded consoles that compiled lakes out for budget.
	// Knob-enabled platforms keep lakes (Old worlds included) through the
	// #if branch above.
#endif

	for (int_t dungeon = 0; dungeon < PLATFORM_POPULATE_DUNGEONS; ++dungeon)
	{
		const int_t x = JavaArithmetic::intAdd(JavaArithmetic::intAdd(blockX, rand.nextInt(16)), 8);
		const int_t y = rand.nextInt(128);
		const int_t z = JavaArithmetic::intAdd(JavaArithmetic::intAdd(blockZ, rand.nextInt(16)), 8);
		WorldGenDungeons().generate(worldObj, rand, x, y, z);
	}

	if (biome != nullptr)
	{
#if PLATFORM_CHUNK_LOCAL_DECORATION
		// Vegetation and ores were written at generation; only the springs are left.
		if (biome->biomeDecorator != nullptr)
		{
			biome->biomeDecorator->beginDecoration(worldObj, rand, blockX, blockZ,
				BiomeDecorator::DecorationPass::Springs);
			try
			{
				while (!biome->biomeDecorator->advanceDecoration())
				{
				}
			}
			catch (...)
			{
				biome->biomeDecorator->finishDecoration();
				throw;
			}
			biome->biomeDecorator->finishDecoration();
		}
#else
		biome->decorate(worldObj, rand, blockX, blockZ);
#endif
#if PLATFORM_POPULATE_WORLDGEN_ANIMALS
		SpawnerAnimals::performWorldGenSpawning(
			worldObj, biome, JavaArithmetic::intAdd(blockX, 8), JavaArithmetic::intAdd(blockZ, 8), 16, 16, rand);
#endif
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
