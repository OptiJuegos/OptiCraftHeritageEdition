#pragma once

#include "java/Type.h"

class World;
class Chunk;

enum class ChunkLoadStatus
{
	Missing,
	Loaded,
	ReadError
};

// net.minecraft.src.IChunkLoader
class IChunkLoader
{
public:
	virtual ~IChunkLoader() = default;
	virtual Chunk *loadChunk(World *world, int_t i, int_t j,
		ChunkLoadStatus *status = nullptr) = 0;
	virtual void saveChunk(World *world, Chunk *chunk) = 0;
	virtual void saveExtraChunkData(World *world, Chunk *chunk) = 0;
	virtual void addRandomArmor() = 0;
	virtual void chunkTick() { addRandomArmor(); }
	virtual void saveExtraData() = 0;
};
