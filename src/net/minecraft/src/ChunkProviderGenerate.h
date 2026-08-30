#pragma once
#include "NoiseBuffer.h"

#include <vector>

#include "IChunkProvider.h"
#include "java/Type.h"
#include "java/Random.h"
#include "java/String.h"
#include "NoiseGeneratorOctaves.h"
#include "platform/PlatformTuning.h"

#if PLATFORM_USE_HEIGHTMAP_TERRAIN
#include "LiteTerrainShape.h"
#endif

class World;
class Chunk;
class BiomeGenBase;
class MapGenBase;
class MapGenMineshaft;
class MapGenVillage;
class MapGenStronghold;
class IProgressUpdate;
class WorldChunkManager;

// net.minecraft.src.ChunkProviderGenerate
class ChunkProviderGenerate : public IChunkProvider
{
public:
	ChunkProviderGenerate(World *world, long_t l, bool mapFeaturesEnabled = true);
	~ChunkProviderGenerate() override;

	void generateTerrain(int_t i, int_t j, byte_t *abyte0, BiomeGenBase **abiomegenbase, const biome_noise_real_t *ad);
#if PLATFORM_USE_HEIGHTMAP_TERRAIN
	// Cheap all-float 2D heightmap terrain path for weak consoles.  Drop-in
	// replacement for generateTerrain(); see ChunkProviderGenerateLite.cpp.
	void generateTerrainHeightmap(int_t i, int_t j, byte_t *abyte0);
	void replaceBlocksForBiomeHeightmap(byte_t *blocks, BiomeGenBase **biomes);
#endif
	void replaceBlocksForBiome(int_t i, int_t j, byte_t *abyte0, BiomeGenBase **abiomegenbase);

	Chunk *prepareChunk(int_t i, int_t j) override;
	Chunk *provideChunk(int_t i, int_t j) override;
	Chunk *loadChunk(int_t i, int_t j) override;

	bool    chunkExists(int_t i, int_t j) override;
	void    populate(IChunkProvider *ichunkprovider, int_t i, int_t j) override;
#if PLATFORM_INCREMENTAL_POPULATE
	bool    populateStep(IChunkProvider *ichunkprovider, int_t i, int_t j) override;
#endif
	bool    saveChunks(bool flag, IProgressUpdate *iprogressupdate) override;
	bool    unload100OldestChunks() override;
	bool    canSave() override;
	jstring makeString() override;
	std::vector<SpawnListEntry> *getPossibleCreatures(const EnumCreatureType &type, int_t x, int_t y, int_t z) override;

	ChunkPosition *findClosestStructure(World *world, const jstring &name, int_t x, int_t y, int_t z) override;

private:
	TerrainNoiseBuffer &initializeNoiseField(TerrainNoiseBuffer &ad, int_t i, int_t j, int_t k,
	                                            int_t l, int_t i1, int_t j1);
	TerrainNoiseBuffer &generateNoiseField(TerrainNoiseBuffer &ad, int_t i, int_t j, int_t k,
	                                        int_t l, int_t i1, int_t j1); // compatibility alias

	// C++ constructs members in declaration order. Keep the Java RNG/noise order:
	// every NoiseGeneratorOctaves constructor consumes rand immediately.
	Random rand;
	NoiseGeneratorOctaves field_912_k;
	NoiseGeneratorOctaves field_911_l;
	NoiseGeneratorOctaves field_910_m;
	NoiseGeneratorOctaves field_909_n;

public:
	NoiseGeneratorOctaves field_922_a;
	NoiseGeneratorOctaves field_921_b;
	NoiseGeneratorOctaves mobSpawnerNoise;

private:
	World *worldObj;
	bool mapFeaturesEnabled;

	TerrainNoiseBuffer field_4180_q;
	TerrainNoiseBuffer stoneNoise;

	MapGenBase *caveGenerator;
	MapGenBase *ravineGenerator;
	MapGenMineshaft *mineshaftGenerator;
	MapGenVillage *villageGenerator;
	MapGenStronghold *strongholdGenerator;

	std::vector<BiomeGenBase *> biomesForGeneration;
#if PLATFORM_USE_HEIGHTMAP_TERRAIN
	std::vector<BiomeGenBase *> liteBiomeHalo;
	std::vector<LiteTerrain::BiomeBlendSample> liteBiomeSamples;
	byte_t liteColumnTop[256] = {};
	byte_t liteTerrainHeight[256] = {};
#endif

	TerrainNoiseBuffer field_4185_d;
	TerrainNoiseBuffer field_4184_e;
	TerrainNoiseBuffer field_4183_f;
	TerrainNoiseBuffer field_4182_g;
	TerrainNoiseBuffer field_4181_h;
	std::vector<float> biomeWeights;
#if PLATFORM_FAST_BIOME_BLEND
	std::vector<float> biomeInverseHeightDenominators;
#endif
	int_t field_914_i[32][32];

	BiomeNoiseBuffer generatedTemperatures;

#if PLATFORM_INCREMENTAL_POPULATE
	enum class PopulateStage
	{
		Structures,
		WaterLake,
		LavaLake,
		Dungeons,
		BiomeDecoration,
		Spawning,
		Snow,
		Done
	};

	struct PopulateTask
	{
		bool active = false;
		int_t chunkX = 0;
		int_t chunkZ = 0;
		int_t blockX = 0;
		int_t blockZ = 0;
		BiomeGenBase *biome = nullptr;
		bool villageGenerated = false;
		bool decorationStarted = false;
		Random random{0};
		PopulateStage stage = PopulateStage::Structures;
		int_t index = 0;
		long long totalNs = 0;
	};

	void beginPopulateTask(int_t i, int_t j);
	bool advancePopulateTask();
	void finishPopulateTask();
	PopulateTask populateTask;
#endif
};
