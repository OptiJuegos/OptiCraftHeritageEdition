#pragma once

#include "IChunkProvider.h"
#include "java/Random.h"

class MapGenVillage;
class World;

// net.minecraft.src.ChunkProviderFlat
class ChunkProviderFlat : public IChunkProvider
{
public:
	ChunkProviderFlat(World *world, long_t seed, bool useStructures);
	~ChunkProviderFlat() override;

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

private:
	void generate(std::vector<byte_t> &blocks);

	World *worldObj;
	Random random;
	bool useStructures;
	MapGenVillage *villageGen;
};
