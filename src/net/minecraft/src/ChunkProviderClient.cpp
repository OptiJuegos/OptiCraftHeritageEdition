#include "ChunkProviderClient.h"

#include <algorithm>
#include <cstdlib>
#include <sstream>

#include "World.h"
#include "Chunk.h"
#include "EmptyChunk.h"
#include "ChunkCoordIntPair.h"
#include "java/String.h"
#include "platform/PlatformTuning.h"

ChunkProviderClient::ChunkProviderClient(World *world)
	: worldObj(world)
	, chunkTopologyVersion(1)
{
	if (PLATFORM_CHUNK_MAP_RESERVE > 0)
	{
		chunkMapping.reserve(PLATFORM_CHUNK_MAP_RESERVE);
		chunkList.reserve(PLATFORM_CHUNK_MAP_RESERVE);
	}
	// The section-based Chunk constructor already represents an empty column.
	// Avoid a transient 32 KiB legacy block array on memory-constrained clients.
	blankChunk = new EmptyChunk(world, 0, 0);
}

ChunkProviderClient::~ChunkProviderClient()
{
	for (auto &entry : chunkMapping)
		delete entry.second;
	delete blankChunk;
}

void ChunkProviderClient::markChunkTopologyChanged()
{
	++chunkTopologyVersion;
	if (chunkTopologyVersion == 0)
		++chunkTopologyVersion;
}

bool ChunkProviderClient::chunkExists(int_t i, int_t j)
{
	#if PLATFORM_MP_DEFERRED_CHUNKS
	// A bounded multiplayer client uses one shared EmptyChunk for every missing
	// column. Reporting that placeholder as loaded lets entities attach to it;
	// when the real chunk arrives they remain indexed in the placeholder and can
	// stop ticking or disappear. Queue them in WorldClient until the real column
	// has been promoted instead.
	return hasChunk(i, j);
	#else
	// Preserve vanilla/desktop ChunkProviderClient behaviour.
	(void)i;
	(void)j;
	return true;
	#endif
}

Chunk *ChunkProviderClient::getChunkIfExists(int_t i, int_t j)
{
#if PLATFORM_MP_DEFERRED_CHUNKS
	auto it = chunkMapping.find(ChunkCoordIntPair::chunkXZ2Long(i, j));
	return it != chunkMapping.end() ? it->second : nullptr;
#else
	return provideChunk(i, j);
#endif
}

void ChunkProviderClient::unloadChunk(int_t i, int_t j)
{
	Chunk *chunk = provideChunk(i, j);
	if (!chunk->isEmptyChunk())
		chunk->onChunkUnload();
	if (chunkMapping.erase(ChunkCoordIntPair::chunkXZ2Long(i, j)) != 0)
		markChunkTopologyChanged();
	chunkList.erase(std::remove(chunkList.begin(), chunkList.end(), chunk), chunkList.end());
	if (chunk != blankChunk)
		delete chunk;
}

bool ChunkProviderClient::hasChunk(int_t i, int_t j) const
{
	return chunkMapping.find(ChunkCoordIntPair::chunkXZ2Long(i, j)) != chunkMapping.end();
}

void ChunkProviderClient::unloadOutsideRadius(int_t centerX, int_t centerZ, int_t radius)
{
	// Collect first: unloadChunk mutates both containers.
	std::vector<Chunk *> outside;
	outside.reserve(chunkMapping.size());
	for (const auto &entry : chunkMapping)
	{
		Chunk *chunk = entry.second;
		if (chunk != nullptr)
		{
			long_t dx = static_cast<long_t>(chunk->xPosition) - static_cast<long_t>(centerX);
			long_t dz = static_cast<long_t>(chunk->zPosition) - static_cast<long_t>(centerZ);
			if (std::llabs(dx) > static_cast<long_t>(radius) ||
			    std::llabs(dz) > static_cast<long_t>(radius))
				outside.push_back(chunk);
		}
	}
	for (Chunk *chunk : outside)
		unloadChunk(chunk->xPosition, chunk->zPosition);
}

Chunk *ChunkProviderClient::prepareChunk(int_t i, int_t j)
{
	const ulong_t key = ChunkCoordIntPair::chunkXZ2Long(i, j);
	auto existing = chunkMapping.find(key);
	if (existing != chunkMapping.end())
	{
		Chunk *old = existing->second;
		if (old != nullptr && old != blankChunk)
			old->onChunkUnload();
		chunkList.erase(std::remove(chunkList.begin(), chunkList.end(), old), chunkList.end());
		chunkMapping.erase(existing);
		markChunkTopologyChanged();
		if (old != blankChunk)
			delete old;
	}

	// Packet51 fills section storage immediately after this call. Constructing
	// through the old 128-high flat block array only allocated and scanned 32 KiB
	// of zeroes before throwing it away.
	Chunk *chunk = new Chunk(worldObj, i, j);
	chunkMapping[key] = chunk;
	markChunkTopologyChanged();
	chunkList.push_back(chunk);
	chunk->isChunkLoaded = true;
	return chunk;
}

Chunk *ChunkProviderClient::provideChunk(int_t i, int_t j)
{
	auto it = chunkMapping.find(ChunkCoordIntPair::chunkXZ2Long(i, j));
	if (it == chunkMapping.end())
		return blankChunk;
	return it->second;
}

bool ChunkProviderClient::saveChunks(bool flag, IProgressUpdate *iprogressupdate)
{
	return true;
}

bool ChunkProviderClient::unload100OldestChunks()
{
	return false;
}

bool ChunkProviderClient::canSave()
{
	return false;
}

void ChunkProviderClient::populate(IChunkProvider *ichunkprovider, int_t i, int_t j)
{
}

jstring ChunkProviderClient::makeString()
{
	return "MultiplayerChunkCache: " + String::fromInt((int_t)chunkMapping.size());
}

void ChunkProviderClient::removeEntityFromLoadedChunks(Entity *entity)
{
	for (const auto &entry : chunkMapping)
	{
		Chunk *chunk = entry.second;
		if (chunk != nullptr && chunk != blankChunk)
			chunk->removeEntity(entity);
	}
}
