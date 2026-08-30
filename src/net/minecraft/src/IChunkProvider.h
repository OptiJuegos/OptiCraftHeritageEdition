#pragma once

#include "java/String.h"
#include "java/Type.h"

#include <vector>

class Chunk;
class Entity;
class IProgressUpdate;
class World;
class ChunkPosition;
class SpawnListEntry;
class EnumCreatureType;

// net.minecraft.src.IChunkProvider
class IChunkProvider
{
public:
	virtual ~IChunkProvider() = default;
	virtual bool chunkExists(int_t i, int_t j) = 0;
	virtual Chunk *provideChunk(int_t i, int_t j) = 0;
	virtual Chunk *prepareChunk(int_t i, int_t j) = 0;
	virtual Chunk *loadChunk(int_t i, int_t j) { return prepareChunk(i, j); }
	virtual void populate(IChunkProvider *ichunkprovider, int_t i, int_t j) = 0;
	virtual bool populateStep(IChunkProvider *ichunkprovider, int_t i, int_t j)
	{
		populate(ichunkprovider, i, j);
		return true;
	}
	virtual bool saveChunks(bool flag, IProgressUpdate *iprogressupdate) = 0;
	virtual bool unload100OldestChunks() = 0;
	virtual bool canSave() = 0;
	virtual jstring makeString() = 0;

	virtual std::vector<SpawnListEntry> *getPossibleCreatures(const EnumCreatureType &type, int_t x, int_t y, int_t z)
	{
		(void)type;
		(void)x;
		(void)y;
		(void)z;
		return nullptr;
	}
	virtual ChunkPosition *findClosestStructure(World *world, const jstring &name, int_t x, int_t y, int_t z)
	{
		(void)world;
		(void)name;
		(void)x;
		(void)y;
		(void)z;
		return nullptr;
	}

	// Numeric counterpart to makeString() for low-memory diagnostics. The
	// default covers generator-only providers that do not own a resident cache.
	virtual int_t getLoadedChunkCount() const { return -1; }
	virtual void removeEntityFromLoadedChunks(Entity *entity) { (void)entity; }
};
