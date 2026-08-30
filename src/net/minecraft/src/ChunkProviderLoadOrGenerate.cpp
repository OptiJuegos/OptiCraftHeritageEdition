#include "ChunkProviderLoadOrGenerate.h"

#include "platform/Log.h"
#include <cstdio>
#include <algorithm>

#include "World.h"
#include "Chunk.h"
#include "IChunkLoader.h"
#include "IProgressUpdate.h"
#include "ThreadedFileIOBase.h"
#include "java/Arithmetic.h"
#include "java/String.h"
#include "java/System.h"
#include "platform/PlatformTuning.h"

#ifdef WII_PLATFORM
// For MC_LOG_LEVEL only; see the extern declaration below.
#include "platform/Profiler.h"
#endif

#if defined(WII_PLATFORM) && MC_LOG_LEVEL >= 2
// Defined in client/Minecraft.cpp; see the matching declaration and comment
// in ChunkProvider.cpp, which feeds the same counter.
#endif

namespace
{
	int_t clampInt(int_t value, int_t minValue, int_t maxValue)
	{
		if (value < minValue) return minValue;
		if (value > maxValue) return maxValue;
		return value;
	}

	// Only the unbounded (desktop) path uses this; PLATFORM_BOUNDED_WORLD takes
	// both radii straight from the tuning table instead of deriving one.
	const int_t CACHE_RADIUS_MARGIN = 4;

	const int_t MAX_UNLOADS_PER_TICK = PLATFORM_MAX_CHUNK_UNLOADS_PER_TICK;
	const long_t MIN_UNUSED_TICKS_BEFORE_UNLOAD = PLATFORM_MIN_UNUSED_TICKS_BEFORE_UNLOAD;
}

ChunkProviderLoadOrGenerate::ChunkProviderLoadOrGenerate()
	: blankChunk(nullptr)
	, chunkProvider(nullptr)
	, chunkLoader(nullptr)
	, worldObj(nullptr)
	, lastQueriedChunkXPos(0)
	, lastQueriedChunkZPos(0)
	, chunks()
	, lastQueriedChunk(nullptr)
	, curChunkX(0)
	, curChunkY(0)
	, chunkLoadRadius(15)
	, chunkUnloadRadius(15)
{
	// Start with enough buckets for normal/short render distances. The map still
	// grows only for chunks that really get loaded, unlike the old fixed 1024 slots.
#if PLATFORM_BOUNDED_WORLD
	chunks.reserve(PLATFORM_CHUNK_MAP_RESERVE);
	setChunkLoadRadius(PLATFORM_CHUNK_CACHE_RADIUS);
#else
	chunks.reserve(256);
#endif
}

ChunkProviderLoadOrGenerate::~ChunkProviderLoadOrGenerate()
{
	for (auto &entry : chunks)
	{
		Chunk *chunk = entry.second;
		if (chunk != nullptr && chunk != blankChunk)
			delete chunk;
	}
	chunks.clear();
	lastQueriedChunk = nullptr;

	delete blankChunk;
	delete chunkLoader;
	delete chunkProvider;

	blankChunk = nullptr;
	chunkLoader = nullptr;
	chunkProvider = nullptr;
	worldObj = nullptr;
}

std::uint64_t ChunkProviderLoadOrGenerate::chunkKey(int_t i, int_t j)
{
	return (static_cast<std::uint64_t>(static_cast<std::uint32_t>(i)) << 32)
	     | static_cast<std::uint32_t>(j);
}

void ChunkProviderLoadOrGenerate::setCurrentChunkOver(int_t i, int_t j)
{
	curChunkX = i;
	curChunkY = j;
}

void ChunkProviderLoadOrGenerate::setChunkLoadRadius(int_t radius)
{
	// Keep a small cache margin around the visible radius. This reduces chunk
	// reload thrashing after the v10 dynamic cache change.
#if PLATFORM_BOUNDED_WORLD
	(void)radius;
	chunkLoadRadius = PLATFORM_CHUNK_CACHE_RADIUS;
	chunkUnloadRadius = PLATFORM_CHUNK_UNLOAD_RADIUS;
#else
	chunkLoadRadius = clampInt(radius, 2, 15);
	chunkUnloadRadius = clampInt(chunkLoadRadius + CACHE_RADIUS_MARGIN, chunkLoadRadius, 15);
#endif
}

void ChunkProviderLoadOrGenerate::setChunkLoadRadiusFromRenderDistance(int_t renderDistance)
{
#if PLATFORM_BOUNDED_WORLD
	(void)renderDistance;
	setChunkLoadRadius(PLATFORM_CHUNK_CACHE_RADIUS);
#else
	renderDistance &= 3;
	int_t blocks = 64 << (3 - renderDistance);
	if (blocks > 400)
		blocks = 400;

	const int_t renderChunksWide = blocks / 16 + 1;
	setChunkLoadRadius(renderChunksWide / 2 + 2);
#endif
}

bool ChunkProviderLoadOrGenerate::canChunkExist(int_t i, int_t j)
{
	const int_t minX = JavaArithmetic::intSub(curChunkX, chunkLoadRadius);
	const int_t minZ = JavaArithmetic::intSub(curChunkY, chunkLoadRadius);
	const int_t maxX = JavaArithmetic::intAdd(curChunkX, chunkLoadRadius);
	const int_t maxZ = JavaArithmetic::intAdd(curChunkY, chunkLoadRadius);
	return i >= minX && j >= minZ && i <= maxX && j <= maxZ;
}

long_t ChunkProviderLoadOrGenerate::currentWorldTime() const
{
	return worldObj != nullptr ? worldObj->getWorldTime() : 0LL;
}

bool ChunkProviderLoadOrGenerate::isOutsideUnloadRadius(int_t i, int_t j) const
{
	const long_t dx = static_cast<long_t>(i) - static_cast<long_t>(curChunkX);
	const long_t dz = static_cast<long_t>(j) - static_cast<long_t>(curChunkY);
	const long_t radius = static_cast<long_t>(chunkUnloadRadius);
	return dx < -radius || dz < -radius || dx > radius || dz > radius;
}

bool ChunkProviderLoadOrGenerate::chunkExists(int_t i, int_t j)
{
	if (!canChunkExist(i, j)) return false;
	if (i == lastQueriedChunkXPos && j == lastQueriedChunkZPos && lastQueriedChunk != nullptr)
		return true;

	auto it = chunks.find(chunkKey(i, j));
	if (it == chunks.end())
		return false;

	Chunk *chunk = it->second;
	return chunk != nullptr && (chunk == blankChunk || chunk->isAtLocation(i, j));
}

Chunk *ChunkProviderLoadOrGenerate::prepareChunk(int_t i, int_t j)
{
	return provideChunk(i, j);
}

Chunk *ChunkProviderLoadOrGenerate::provideChunk(int_t i, int_t j)
{
#if PLATFORM_BOUNDED_WORLD
	if (worldObj != nullptr && !worldObj->findingSpawnPoint && !canChunkExist(i, j))
		return blankChunk;
#endif
	if (i == lastQueriedChunkXPos && j == lastQueriedChunkZPos && lastQueriedChunk != nullptr)
	{
		// Field write on an already-resolved pointer -- no chunkKey hash, no map
		// probe. This used to defeat its own purpose by writing through a side
		// unordered_map keyed by coordinate; see Chunk::lastAccessTick.
		lastQueriedChunk->lastAccessTick = currentWorldTime();
		return lastQueriedChunk;
	}

	if (worldObj != nullptr && !worldObj->findingSpawnPoint && !canChunkExist(i, j))
		return blankChunk;

	const std::uint64_t key = chunkKey(i, j);
	auto it = chunks.find(key);
	if (it != chunks.end())
	{
		Chunk *cached = it->second;
		if (cached != nullptr && (cached == blankChunk || cached->isAtLocation(i, j)))
		{
			lastQueriedChunkXPos = i;
			lastQueriedChunkZPos = j;
			lastQueriedChunk = cached;
			cached->lastAccessTick = currentWorldTime();
			return cached;
		}

		// Defensive cleanup: this should not happen with chunkKey(), but avoids
		// retaining a stale slot if a corrupted entry got inserted earlier.
		unloadChunk(it->first, cached);
		chunks.erase(it);
	}

	bool readFailed = false;
	Chunk *chunk = loadChunkFromFile(i, j, readFailed);
	if (chunk == nullptr && readFailed)
	{
		MC_LOG_ERROR("chunk", "ChunkProviderLoadOrGenerate: refusing to regenerate unreadable chunk %d,%d\n", i, j);
		chunk = blankChunk;
	}
	else if (chunk == nullptr || chunk == blankChunk)
	{
		if (chunkProvider == nullptr)
			chunk = blankChunk;
		else
		{
			chunk = chunkProvider->provideChunk(i, j);
			if (chunk != nullptr && chunk != blankChunk)
				chunk->remapBlocks();
		}
	}

	if (chunk == nullptr)
		chunk = blankChunk;

	chunks[key] = chunk;
	chunk->lastAccessTick = currentWorldTime();

	if (chunk != nullptr)
	{
		chunk->onChunkLoadData();
		chunk->onChunkLoad();
	}

	// Populate-readiness block. Every condition is gated on chunkExists() first,
	// which on low consoles returns false for not-yet-generated chunks WITHOUT
	// forcing generation (it short-circuits before provideChunk). So a chunk only
	// gets decorated once its +1 neighbors already exist -- no generation cascade.
	if (chunk != nullptr && chunk != blankChunk)
	{
		const int_t eastX = JavaArithmetic::intAdd(i, 1);
		const int_t westX = JavaArithmetic::intSub(i, 1);
		const int_t southZ = JavaArithmetic::intAdd(j, 1);
		const int_t northZ = JavaArithmetic::intSub(j, 1);
		if (!chunk->isTerrainPopulated
			&& chunkExists(eastX, southZ)
			&& chunkExists(i, southZ)
			&& chunkExists(eastX, j))
		{
			populate(this, i, j);
		}
		if (chunkExists(westX, j) && !provideChunk(westX, j)->isTerrainPopulated
			&& chunkExists(westX, southZ)
			&& chunkExists(i, southZ)
			&& chunkExists(westX, j))
		{
			populate(this, westX, j);
		}
		if (chunkExists(i, northZ) && !provideChunk(i, northZ)->isTerrainPopulated
			&& chunkExists(eastX, northZ)
			&& chunkExists(i, northZ)
			&& chunkExists(eastX, j))
		{
			populate(this, i, northZ);
		}
		if (chunkExists(westX, northZ) && !provideChunk(westX, northZ)->isTerrainPopulated
			&& chunkExists(westX, northZ)
			&& chunkExists(i, northZ)
			&& chunkExists(westX, j))
		{
			populate(this, westX, northZ);
		}
	}

	lastQueriedChunkXPos = i;
	lastQueriedChunkZPos = j;
	lastQueriedChunk = chunk;
	return chunk;
}

Chunk *ChunkProviderLoadOrGenerate::loadChunkFromFile(int_t i, int_t j, bool &readFailed)
{
	readFailed = false;
	if (chunkLoader == nullptr) return blankChunk;
	try
	{
		ChunkLoadStatus status = ChunkLoadStatus::Missing;
		Chunk *chunk = chunkLoader->loadChunk(worldObj, i, j, &status);
		readFailed = status == ChunkLoadStatus::ReadError;
		if (chunk != nullptr)
			chunk->lastSaveTime = worldObj->getWorldTime();
		return chunk;
	}
	catch (...)
	{
		readFailed = true;
		MC_LOG_ERROR("chunk", "ChunkProviderLoadOrGenerate::loadChunkFromFile - exception %d,%d\n", i, j);
	}
	return blankChunk;
}

void ChunkProviderLoadOrGenerate::saveExtraChunkData(Chunk *chunk)
{
	if (chunkLoader == nullptr || chunk == nullptr || chunk == blankChunk) return;
	try
	{
		chunkLoader->saveExtraChunkData(worldObj, chunk);
	}
	catch (...)
	{
		MC_LOG_ERROR("chunk", "ChunkProviderLoadOrGenerate::saveExtraChunkData - exception\n");
	}
}

void ChunkProviderLoadOrGenerate::saveChunkToFile(Chunk *chunk)
{
	if (chunkLoader == nullptr || chunk == nullptr || chunk == blankChunk) return;
	try
	{
		chunk->lastSaveTime = worldObj->getWorldTime();
		chunkLoader->saveChunk(worldObj, chunk);
	}
	catch (...)
	{
		MC_LOG_ERROR("chunk", "ChunkProviderLoadOrGenerate::saveChunkToFile - exception\n");
	}
}

void ChunkProviderLoadOrGenerate::unloadChunk(std::uint64_t key, Chunk *chunk)
{
	(void)key; // no longer needed: last-access tracking moved onto Chunk itself
	if (chunk == nullptr || chunk == blankChunk)
		return;

#if PLATFORM_SAVE_RUNTIME_CHUNK_EDITS_ON_UNLOAD
	if (!chunk->neverSave && chunk->isRuntimeSaveRequired())
	{
		saveChunkToFile(chunk);
		chunk->isModified = false;
		chunk->clearRuntimeSaveRequired();
		saveExtraChunkData(chunk);
	}
#elif !PLATFORM_CONSOLE_LOW
	if (!chunk->neverSave && chunk->needsSaving(false))
	{
#if defined(WII_PLATFORM) && MC_LOG_LEVEL >= 2
		const long_t wiiUnloadSaveStartNs = System::nanoTime();
#endif
		saveChunkToFile(chunk);
		chunk->isModified = false;
		chunk->clearRuntimeSaveRequired();
		saveExtraChunkData(chunk);
#if defined(WII_PLATFORM) && MC_LOG_LEVEL >= 2
		platformProfileUnloadSave(System::nanoTime() - wiiUnloadSaveStartNs);
#endif
	}
#endif

	chunk->onChunkUnload();

	if (lastQueriedChunk == chunk)
	{
		lastQueriedChunk = nullptr;
		lastQueriedChunkXPos = 0;
		lastQueriedChunkZPos = 0;
	}

	delete chunk;
}

void ChunkProviderLoadOrGenerate::populate(IChunkProvider *ichunkprovider, int_t i, int_t j)
{
	Chunk *chunk = provideChunk(i, j);
	if (chunk != nullptr && chunk != blankChunk && !chunk->isTerrainPopulated)
	{
		chunk->isTerrainPopulated = true;
		if (chunkProvider != nullptr)
		{
			chunkProvider->populate(ichunkprovider, i, j);
			chunk->setChunkModified();
		}
	}
}

bool ChunkProviderLoadOrGenerate::saveChunks(bool flag, IProgressUpdate *iprogressupdate)
{
	// Chunk persistence is enabled on console now (region-file backed). This only
	// runs on an explicit full save (flag = true) because runtime autosave is
	// disabled (PLATFORM_DISABLE_RUNTIME_AUTOSAVE) -- it is not a per-frame cost.
	int_t saved = 0;
	int_t totalToSave = 0;
	if (iprogressupdate != nullptr)
	{
		for (auto &entry : chunks)
		{
			Chunk *chunk = entry.second;
			if (chunk != nullptr && chunk != blankChunk && chunk->needsSaving(flag))
				totalToSave++;
		}
	}

	int_t progress = 0;
	for (auto &entry : chunks)
	{
		Chunk *chunk = entry.second;
		if (chunk == nullptr || chunk == blankChunk) continue;
		if (flag && !chunk->neverSave)
			saveExtraChunkData(chunk);
		if (!chunk->needsSaving(flag)) continue;
		saveChunkToFile(chunk);
		chunk->isModified = false;
		chunk->clearRuntimeSaveRequired();
		++saved;
		if (flag && (saved % std::max<int_t>(1, PLATFORM_INCREMENTAL_CHUNK_SAVE_LIMIT)) == 0)
			ThreadedFileIOBase::threadedIOInstance.waitForFinish();
		if (saved == PLATFORM_INCREMENTAL_CHUNK_SAVE_LIMIT && !flag)
			return false;
		if (iprogressupdate != nullptr && totalToSave > 0 && ++progress % 10 == 0)
			iprogressupdate->setLoadingProgress((progress * 100) / totalToSave);
	}
	if (flag)
	{
		if (chunkLoader == nullptr) return true;
		chunkLoader->saveExtraData();
	}
	return true;
}

bool ChunkProviderLoadOrGenerate::unload100OldestChunks()
{
    if (chunkLoader != nullptr)
        chunkLoader->chunkTick();

    int_t unloaded = 0;
    const long_t now = currentWorldTime();
    const size_t max_chunks = (size_t)((chunkUnloadRadius * 2 + 1) * (chunkUnloadRadius * 2 + 1));
    
    // EMERGENCY UNLOAD: if we have way more chunks than the unload radius allows,
    // ignore the time gate and drain outside chunks at a bounded accelerated rate. This prevents
    // OOM when loading a saved world with many chunks or when RTTI fails.
    size_t chunksOutsideRadius = 0;
    for (const auto &entry : chunks)
    {
        Chunk *chunk = entry.second;
        if (chunk != nullptr && chunk != blankChunk
            && isOutsideUnloadRadius(chunk->xPosition, chunk->zPosition))
        {
            chunksOutsideRadius++;
        }
    }
    bool emergency = chunks.size() > max_chunks && chunksOutsideRadius > 0;
    const int_t unloadLimit = emergency
        ? PLATFORM_EMERGENCY_CHUNK_UNLOADS_PER_TICK
        : MAX_UNLOADS_PER_TICK;

    for (auto it = chunks.begin(); it != chunks.end(); )
    {
        Chunk *chunk = it->second;
        if (chunk == nullptr || chunk == blankChunk)
        {
            it = chunks.erase(it);
            continue;
        }

        const long_t lastAccess = chunk->lastAccessTick;

        if (isOutsideUnloadRadius(chunk->xPosition, chunk->zPosition)
            && (emergency || JavaArithmetic::longSub(now, lastAccess) >= MIN_UNUSED_TICKS_BEFORE_UNLOAD))
        {
            unloadChunk(it->first, chunk);
            it = chunks.erase(it);
            if (chunksOutsideRadius > 0)
                chunksOutsideRadius--;
            if (++unloaded >= unloadLimit)
                break;
            if (emergency && chunksOutsideRadius == 0)
                break;
        }
        else
        {
            ++it;
        }
    }

    const bool childUnloaded = chunkProvider != nullptr && chunkProvider->unload100OldestChunks();
    return unloaded > 0 || childUnloaded;
}

bool ChunkProviderLoadOrGenerate::canSave()
{
	return true;
}

jstring ChunkProviderLoadOrGenerate::makeString()
{
	return "ChunkCache: " + String::fromInt((int_t)chunks.size())
		+ " Radius: " + String::fromInt(chunkLoadRadius)
		+ " UnloadRadius: " + String::fromInt(chunkUnloadRadius);
}

void ChunkProviderLoadOrGenerate::removeEntityFromLoadedChunks(Entity *entity)
{
	for (const auto &entry : chunks)
	{
		Chunk *chunk = entry.second;
		if (chunk != nullptr && chunk != blankChunk)
			chunk->removeEntityFromAllSections(entity);
	}
}
