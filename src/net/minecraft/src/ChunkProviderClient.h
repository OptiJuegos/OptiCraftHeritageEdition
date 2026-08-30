#pragma once

#include <unordered_map>
#include <vector>

#include "IChunkProvider.h"
#include "java/Type.h"
#include "java/String.h"

class World;
class Chunk;
class IProgressUpdate;

// net.minecraft.src.ChunkProviderClient
class ChunkProviderClient : public IChunkProvider
{
public:
	explicit ChunkProviderClient(World *world);
	~ChunkProviderClient() override;

	bool    chunkExists(int_t i, int_t j) override;
	void    unloadChunk(int_t i, int_t j);   // func_539_c
	bool    hasChunk(int_t i, int_t j) const;
	void    unloadOutsideRadius(int_t centerX, int_t centerZ, int_t radius);
	Chunk  *prepareChunk(int_t i, int_t j) override;
	Chunk  *provideChunk(int_t i, int_t j) override;

	bool    saveChunks(bool flag, IProgressUpdate *iprogressupdate) override;
	bool    unload100OldestChunks() override;
	bool    canSave() override;
	void    populate(IChunkProvider *ichunkprovider, int_t i, int_t j) override;
	jstring makeString() override;
	int_t   getLoadedChunkCount() const override { return (int_t)chunkMapping.size(); }
	void    removeEntityFromLoadedChunks(Entity *entity) override;

private:
	Chunk *blankChunk;
	std::unordered_map<ulong_t, Chunk *> chunkMapping;
	std::vector<Chunk *> chunkList;   // field_889_c
	World *worldObj;
};
