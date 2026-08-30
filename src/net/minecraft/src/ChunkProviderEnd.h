#pragma once

#include <vector>

#include "IChunkProvider.h"
#include "NoiseBuffer.h"
#include "NoiseGeneratorOctaves.h"
#include "java/Random.h"

class BiomeGenBase;
class World;

// net.minecraft.src.ChunkProviderEnd
class ChunkProviderEnd : public IChunkProvider
{
public:
	ChunkProviderEnd(World *world, long_t seed);

	Chunk *prepareChunk(int_t x, int_t z) override;
	Chunk *loadChunk(int_t x, int_t z) override;
	Chunk *provideChunk(int_t x, int_t z) override;
	bool chunkExists(int_t x, int_t z) override;
	void populate(IChunkProvider *provider, int_t x, int_t z) override;
	bool saveChunks(bool saveAll, IProgressUpdate *progress) override;
	bool unload100OldestChunks() override;
	bool canSave() override;
	jstring makeString() override;
	std::vector<SpawnListEntry> *getPossibleCreatures(const EnumCreatureType &type, int_t x, int_t y, int_t z) override;
	ChunkPosition *findClosestStructure(World *world, const jstring &name, int_t x, int_t y, int_t z) override;

	void generateTerrain(int_t chunkX, int_t chunkZ, byte_t *blocks, const std::vector<BiomeGenBase *> &biomes);
	void replaceBlocksForBiome(int_t chunkX, int_t chunkZ, byte_t *blocks, const std::vector<BiomeGenBase *> &biomes);

private:
	Random endRNG;
	NoiseGeneratorOctaves noiseGen1;
	NoiseGeneratorOctaves noiseGen2;
	NoiseGeneratorOctaves noiseGen3;

public:
	NoiseGeneratorOctaves noiseGen4;
	NoiseGeneratorOctaves noiseGen5;

private:
	TerrainNoiseBuffer &initializeNoiseField(TerrainNoiseBuffer &values, int_t x, int_t y, int_t z,
	                                        int_t sizeX, int_t sizeY, int_t sizeZ);

	World *endWorld;
	TerrainNoiseBuffer densities;
	std::vector<BiomeGenBase *> biomesForGeneration;
	TerrainNoiseBuffer noiseData1;
	TerrainNoiseBuffer noiseData2;
	TerrainNoiseBuffer noiseData3;
	TerrainNoiseBuffer noiseData4;
	TerrainNoiseBuffer noiseData5;
};
