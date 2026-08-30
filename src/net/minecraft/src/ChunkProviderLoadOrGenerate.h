#pragma once

#include <unordered_map>

#include <cstdint>

#include "IChunkProvider.h"
#include "java/Type.h"
#include "java/String.h"

class World;
class Chunk;
class IChunkLoader;
class IProgressUpdate;

// net.minecraft.src.ChunkProviderLoadOrGenerate
class ChunkProviderLoadOrGenerate : public IChunkProvider
{
public:
	ChunkProviderLoadOrGenerate();
	~ChunkProviderLoadOrGenerate() override;

	void setCurrentChunkOver(int_t i, int_t j);
	void setChunkLoadRadius(int_t radius);
	void setChunkLoadRadiusFromRenderDistance(int_t renderDistance);
	bool canChunkExist(int_t i, int_t j);

	bool   chunkExists(int_t i, int_t j) override;
	Chunk *prepareChunk(int_t i, int_t j) override;
	Chunk *provideChunk(int_t i, int_t j) override;
	void   populate(IChunkProvider *ichunkprovider, int_t i, int_t j) override;
	bool   saveChunks(bool flag, IProgressUpdate *iprogressupdate) override;
	bool   unload100OldestChunks() override;
	bool   canSave() override;
	jstring makeString() override;
	int_t  getLoadedChunkCount() const override { return (int_t)chunks.size(); }
	void   removeEntityFromLoadedChunks(Entity *entity) override;

	Chunk          *blankChunk;
	IChunkProvider *chunkProvider;
	IChunkLoader   *chunkLoader;
	World          *worldObj;
	int_t           lastQueriedChunkXPos;
	int_t           lastQueriedChunkZPos;

private:
	static std::uint64_t chunkKey(int_t i, int_t j);

	Chunk *loadChunkFromFile(int_t i, int_t j, bool &readFailed);  // func_542_c
	void   saveExtraChunkData(Chunk *chunk);
	void   saveChunkToFile(Chunk *chunk);
	void   unloadChunk(std::uint64_t key, Chunk *chunk);
	long_t currentWorldTime() const;
	bool   isOutsideUnloadRadius(int_t i, int_t j) const;

	std::unordered_map<std::uint64_t, Chunk *> chunks;
	Chunk *lastQueriedChunk;
	int_t  curChunkX;
	int_t  curChunkY;
	int_t  chunkLoadRadius;
	int_t  chunkUnloadRadius;
};
