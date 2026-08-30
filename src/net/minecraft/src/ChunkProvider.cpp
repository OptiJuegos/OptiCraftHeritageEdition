#include "platform/Log.h"
#include "ChunkProvider.h"

#include <cstdio>
#include <algorithm>
#include <sstream>

#include "World.h"
#include "Chunk.h"
#include "EmptyChunk.h"
#include "IChunkLoader.h"
#include "IProgressUpdate.h"
#include "ThreadedFileIOBase.h"
#include "ChunkCoordIntPair.h"
#include "ChunkProviderLoadOrGenerate.h"
#include "ChunkProviderGenerate.h"
#include "McRegionChunkLoader.h"
#include "java/Arithmetic.h"
#include "java/String.h"
#include "java/System.h"
#include "platform/PlatformTuning.h"
#include "platform/WorldLoadTrace.h"
#include "platform/chunks/ChunkGenerationScheduler.h"

#include "platform/Profiler.h"

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

#if PLATFORM_BOUNDED_WORLD
	const int_t POPULATION_FOOTPRINT_AXIS = 2;
	const int_t POPULATION_SECTION_COUNT = 8;
#endif
}

ChunkProvider::ChunkProvider(World *world, IChunkLoader *ichunkloader, IChunkProvider *ichunkprovider)
#if PLATFORM_BOUNDED_WORLD && PLATFORM_ASYNC_CHUNK_GENERATION
	: asyncGenerationScheduler(nullptr)
	, droppedChunksSet()
#else
	: droppedChunksSet()
#endif
	, blankChunk(nullptr)
	, chunkProvider(ichunkprovider)
	, chunkLoader(ichunkloader)
	, chunkMap()
	, chunkList()
	, lastChunk(nullptr)
	, lastChunkX(0)
	, lastChunkZ(0)
	, worldObj(world)
	, curChunkX(0)
	, curChunkZ(0)
	, chunkLoadRadius(15)
	, chunkUnloadRadius(15)
{
	blankChunk = new EmptyChunk(world, std::vector<byte_t>(32768, 0), 0, 0);
#if PLATFORM_BOUNDED_WORLD
	chunkMap.reserve(PLATFORM_CHUNK_MAP_RESERVE);
	chunkList.reserve(PLATFORM_CHUNK_MAP_RESERVE);
	genChunksThisTick = 0;
	setChunkLoadRadius(PLATFORM_CHUNK_CACHE_RADIUS);
#if PLATFORM_ASYNC_CHUNK_GENERATION
	if (dynamic_cast<ChunkProviderGenerate *>(chunkProvider) != nullptr)
	{
		asyncGenerationScheduler = new ChunkGenerationScheduler(
			new ChunkProviderGenerate(worldObj, worldObj->getRandomSeed()),
			dynamic_cast<McRegionChunkLoader *>(ichunkloader));
		if (!asyncGenerationScheduler->start())
		{
			delete asyncGenerationScheduler;
			asyncGenerationScheduler = nullptr;
		}
	}
#endif
#else
	chunkMap.reserve(256);
	chunkList.reserve(256);
#endif
}

ChunkProvider::~ChunkProvider()
{
#if PLATFORM_BOUNDED_WORLD && PLATFORM_ASYNC_CHUNK_GENERATION
	delete asyncGenerationScheduler;
	asyncGenerationScheduler = nullptr;
#endif
	std::unordered_set<Chunk *> uniqueChunks;
	uniqueChunks.reserve(chunkMap.size());
	for (auto &entry : chunkMap)
	{
		Chunk *chunk = entry.second;
		if (chunk != nullptr && chunk != blankChunk)
			uniqueChunks.insert(chunk);
	}

	for (Chunk *chunk : uniqueChunks)
		delete chunk;

	chunkMap.clear();
	chunkList.clear();
	droppedChunksSet.clear();

	delete blankChunk;
	delete chunkLoader;
	delete chunkProvider;

	blankChunk = nullptr;
	chunkLoader = nullptr;
	chunkProvider = nullptr;
	worldObj = nullptr;
}

std::uint64_t ChunkProvider::chunkKey(int_t i, int_t j)
{
	return (static_cast<std::uint64_t>(static_cast<std::uint32_t>(i)) << 32)
	     | static_cast<std::uint32_t>(j);
}

void ChunkProvider::setCurrentChunkOver(int_t i, int_t j)
{
	curChunkX = i;
	curChunkZ = j;

	// Propagate the player-centred unload origin to the inner provider. Without
	// this the inner ChunkProviderLoadOrGenerate keeps curChunkX/Z at its initial
	// (spawn) value, so its distance-based unload100OldestChunks() never evicts the
	// chunks it generates around the moving player -- they accumulate and leak RAM
	// as you walk (the 32 MB "OOM after ~30 s of moving"). Both cache tiers must be
	// centred on the player. dynamic_cast because setCurrentChunkOver is not on the
	// IChunkProvider interface.
	if (ChunkProviderLoadOrGenerate *inner = dynamic_cast<ChunkProviderLoadOrGenerate *>(chunkProvider))
		inner->setCurrentChunkOver(i, j);
	else if (ChunkProvider *inner2 = dynamic_cast<ChunkProvider *>(chunkProvider))
		inner2->setCurrentChunkOver(i, j);
}

#if PLATFORM_BOUNDED_WORLD
void ChunkProvider::notifyChunkPublished(Chunk *chunk)
{
	if (worldObj == nullptr || chunk == nullptr || chunk == blankChunk)
		return;

	// A console renderer at the edge of the sliding world cache is allowed to
	// build against EmptyChunk for source chunks outside the current load radius.
	// If that chunk later becomes resident, its completed empty column is stale.
	// Chunk publication is the authoritative transition, so invalidate that
	// column here. Use the interior range because RenderGlobal expands dirty
	// ranges by one block; this reaches the chunk bounds without rebuilding all
	// eight horizontal neighbours for every streamed chunk. Active neighbouring
	// builds detect source availability changes themselves in WorldRenderer.
	const int_t minX = JavaArithmetic::intMul(chunk->xPosition, 16);
	const int_t minZ = JavaArithmetic::intMul(chunk->zPosition, 16);
	worldObj->markBlocksDirty(minX + 1, 1, minZ + 1,
	                          minX + 14, 126, minZ + 14);
#if PLATFORM_PS2
	worldObj->notifyChunkPublishedForRender(chunk->xPosition, chunk->zPosition);
#endif
}
#endif

void ChunkProvider::setChunkLoadRadius(int_t radius)
{
	// Use a slightly larger RAM cache than the strict visible radius.  This avoids
	// unloading/reloading the same chunks when the player moves a few blocks,
	// without going back to the old fixed 1024-slot cache.
#if PLATFORM_BOUNDED_WORLD
	(void)radius;
	chunkLoadRadius = PLATFORM_CHUNK_CACHE_RADIUS;
	chunkUnloadRadius = PLATFORM_CHUNK_UNLOAD_RADIUS;
#else
	chunkLoadRadius = clampInt(radius, 2, 15);
	chunkUnloadRadius = clampInt(chunkLoadRadius + CACHE_RADIUS_MARGIN, chunkLoadRadius, 15);
#endif
}

void ChunkProvider::setChunkLoadRadiusFromRenderDistance(int_t renderDistance)
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

bool ChunkProvider::canChunkExist(int_t i, int_t j) const
{
	const int_t minX = JavaArithmetic::intSub(curChunkX, chunkLoadRadius);
	const int_t minZ = JavaArithmetic::intSub(curChunkZ, chunkLoadRadius);
	const int_t maxX = JavaArithmetic::intAdd(curChunkX, chunkLoadRadius);
	const int_t maxZ = JavaArithmetic::intAdd(curChunkZ, chunkLoadRadius);
	return i >= minX && j >= minZ && i <= maxX && j <= maxZ;
}

long_t ChunkProvider::currentWorldTime() const
{
	return worldObj != nullptr ? worldObj->getWorldTime() : 0LL;
}

bool ChunkProvider::isOutsideUnloadRadius(int_t i, int_t j) const
{
	const long_t dx = static_cast<long_t>(i) - static_cast<long_t>(curChunkX);
	const long_t dz = static_cast<long_t>(j) - static_cast<long_t>(curChunkZ);
	const long_t radius = static_cast<long_t>(chunkUnloadRadius);
	return dx < -radius || dz < -radius || dx > radius || dz > radius;
}

bool ChunkProvider::chunkExists(int_t i, int_t j)
{
	return chunkMap.count(chunkKey(i, j)) != 0;
}

#if PLATFORM_BOUNDED_WORLD && PLATFORM_ASYNC_CHUNK_GENERATION
ChunkProvider::ChunkRequestStatus ChunkProvider::requestChunkDetailed(int_t i, int_t j)
{
	if (asyncGenerationScheduler == nullptr || !asyncGenerationScheduler->active())
		return ChunkRequestStatus::Inactive;
	if (worldObj != nullptr && !worldObj->findingSpawnPoint && !canChunkExist(i, j))
		return ChunkRequestStatus::OutOfRange;
	if (chunkMap.count(chunkKey(i, j)) != 0)
		return ChunkRequestStatus::AlreadyLoaded;

	switch (asyncGenerationScheduler->requestDetailed(i, j, PLATFORM_ASYNC_GENERATION_QUEUE_LIMIT))
	{
	case ChunkGenerationScheduler::RequestStatus::Accepted:
		return ChunkRequestStatus::Accepted;
	case ChunkGenerationScheduler::RequestStatus::AlreadyQueued:
		return ChunkRequestStatus::AlreadyQueued;
	case ChunkGenerationScheduler::RequestStatus::QueueFull:
		return ChunkRequestStatus::QueueFull;
	case ChunkGenerationScheduler::RequestStatus::Inactive:
	default:
		return ChunkRequestStatus::Inactive;
	}
}

bool ChunkProvider::requestChunk(int_t i, int_t j)
{
	return requestChunkDetailed(i, j) == ChunkRequestStatus::Accepted;
}

void ChunkProvider::serviceAsyncChunkStreaming()
{
	if (asyncGenerationScheduler == nullptr || !asyncGenerationScheduler->active())
		return;

	if (PLATFORM_ASYNC_GENERATION_PUBLISH_PER_FRAME > 0)
		drainAsyncGeneratedChunks(PLATFORM_ASYNC_GENERATION_PUBLISH_PER_FRAME);
	if (PLATFORM_ASYNC_GENERATION_REQUESTS_PER_FRAME > 0)
		drainAsyncGenerationRequests(PLATFORM_ASYNC_GENERATION_REQUESTS_PER_FRAME);
}

bool ChunkProvider::acceptAsyncGenerationCoordinate(void* context, int_t i, int_t j)
{
	ChunkProvider* self = static_cast<ChunkProvider*>(context);
	if (self == nullptr)
		return false;
	if (self->chunkMap.count(chunkKey(i, j)) != 0)
		return false;
	return self->worldObj == nullptr || self->worldObj->findingSpawnPoint || self->canChunkExist(i, j);
}

bool ChunkProvider::drainAsyncGenerationRequests(int_t budget)
{
	return asyncGenerationScheduler != nullptr &&
		asyncGenerationScheduler->dispatch(budget, &ChunkProvider::acceptAsyncGenerationCoordinate, this);
}

bool ChunkProvider::drainAsyncGeneratedChunks(int_t budget)
{
	if (asyncGenerationScheduler == nullptr || budget <= 0)
		return false;

	bool published = false;
	for (int_t n = 0; n < budget; ++n)
	{
		ChunkGenerationScheduler::Result result;
		if (!asyncGenerationScheduler->popResult(result))
			break;

		const std::uint64_t key = chunkKey(result.x, result.z);
		const bool wanted = chunkMap.count(key) == 0
			&& (worldObj == nullptr || worldObj->findingSpawnPoint || canChunkExist(result.x, result.z));

		Chunk *chunk = nullptr;
		if (wanted)
		{
			switch (result.kind)
			{
			case ChunkGenerationScheduler::ResultKind::LoadedData:
			{
				McRegionChunkLoader* regionLoader = dynamic_cast<McRegionChunkLoader*>(chunkLoader);
				if (regionLoader != nullptr)
				{
					ChunkLoadStatus loadStatus = ChunkLoadStatus::ReadError;
					chunk = regionLoader->loadChunkFromData(worldObj, result.x, result.z, result.data, &loadStatus);
					if (chunk != nullptr)
						chunk->lastSaveTime = currentWorldTime();
					else if (loadStatus == ChunkLoadStatus::ReadError)
						chunk = blankChunk;
				}
				break;
			}
			case ChunkGenerationScheduler::ResultKind::Generated:
				chunk = result.chunk;
				result.chunk = nullptr;
				break;
			case ChunkGenerationScheduler::ResultKind::ReadError:
				chunk = blankChunk;
				break;
			}
		}

		if (chunk != nullptr)
		{
			published = true;
			chunkMap[key] = chunk;
			chunkList.push_back(chunk);
			chunk->lastAccessTick = currentWorldTime();
			if (chunk != blankChunk)
			{
				chunk->onChunkLoadData();
				chunk->onChunkLoad();
				notifyChunkPublished(chunk);
				const int_t westX = JavaArithmetic::intSub(result.x, 1);
				const int_t northZ = JavaArithmetic::intSub(result.z, 1);
				enqueuePopulate(result.x, result.z);
				enqueuePopulate(westX, result.z);
				enqueuePopulate(result.x, northZ);
				enqueuePopulate(westX, northZ);
			}
		}

		delete result.chunk;
		asyncGenerationScheduler->complete(result.x, result.z);
	}
	return published;
}
#endif

Chunk *ChunkProvider::prepareChunk(int_t i, int_t j)
{
#if PLATFORM_BOUNDED_WORLD
	if (worldObj != nullptr && !worldObj->findingSpawnPoint && !canChunkExist(i, j))
		return blankChunk;
#endif
	const std::uint64_t key = chunkKey(i, j);
	droppedChunksSet.erase(key);

	auto it = chunkMap.find(key);
	Chunk *chunk = (it != chunkMap.end()) ? it->second : nullptr;

	if (chunk == nullptr)
	{
		WORLD_LOAD_STAGE("prepareChunk");
		bool readFailed = false;
#if PLATFORM_PROFILE_STREAMING
		const long_t chunkLoadStartNs = System::nanoTime();
#endif
		WorldLoadTrace::step("loadChunkFromFile");
		chunk = loadChunkFromFile(i, j, readFailed);
#if PLATFORM_PROFILE_STREAMING
		platformProfileChunkLoad(System::nanoTime() - chunkLoadStartNs);
#endif
		if (chunk == nullptr && readFailed)
		{
			MC_LOG_ERROR("chunk", "ChunkProvider: refusing to regenerate unreadable chunk %d,%d\n", i, j);
			chunk = blankChunk;
		}
		else if (chunk == nullptr)
		{
			if (chunkProvider == nullptr)
				chunk = blankChunk;
			else
			{
#if PLATFORM_PROFILE_STREAMING
				const long_t generateStartNs = System::nanoTime();
#endif
				WorldLoadTrace::step("generate");
				chunk = chunkProvider->provideChunk(i, j);
#if PLATFORM_PROFILE_STREAMING
				platformProfileGenerate(System::nanoTime() - generateStartNs);
#endif
			}
		}
		if (chunk == nullptr)
			chunk = blankChunk;

		chunkMap[key] = chunk;
		chunkList.push_back(chunk);

		if (chunk != nullptr)
		{
			chunk->onChunkLoadData();
			chunk->onChunkLoad();
		}
#if PLATFORM_BOUNDED_WORLD
		if (chunk != nullptr && chunk != blankChunk)
			notifyChunkPublished(chunk);
#endif

		// Drives populate() (trees, ores, lakes, flowers, snow) once a chunk's
		// +1 neighbors exist. Every branch is gated on chunkExists() first, which
		// only consults the map (never generates), and populate(i,j) only writes
		// into the 2x2 block of chunks all required here -- so it never forces an
		// ungenerated neighbor and cannot trigger a generation cascade. The old
		// ~850ms spike came from the separate 5x5/prefetch force-gen paths, now
		// fixed; the remaining cost is the decoration work itself.
		if (chunk != nullptr && chunk != blankChunk)
		{
			const int_t eastX = JavaArithmetic::intAdd(i, 1);
			const int_t westX = JavaArithmetic::intSub(i, 1);
			const int_t southZ = JavaArithmetic::intAdd(j, 1);
			const int_t northZ = JavaArithmetic::intSub(j, 1);
#if PLATFORM_BOUNDED_WORLD
			// Defer decoration off the generation frame. Creating (i,j) can unlock
			// populate-readiness for itself and its three lower/left neighbours;
			// enqueue all four and let the per-tick drain re-check the gate and
			// decorate within budget. Enqueuing (never populating inline here) is
			// what breaks the populate->setBlock->provideChunk re-entrancy.
			enqueuePopulate(i, j);
			enqueuePopulate(westX, j);
			enqueuePopulate(i, northZ);
			enqueuePopulate(westX, northZ);
#else
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
#endif
		}
	}

	chunk->lastAccessTick = currentWorldTime();
	return chunk;
}

Chunk *ChunkProvider::provideChunk(int_t i, int_t j)
{
#if PLATFORM_BOUNDED_WORLD
	if (worldObj != nullptr && !worldObj->findingSpawnPoint && !canChunkExist(i, j))
		return blankChunk;
#endif
	// Fast path: same chunk as the previous lookup. Skips the chunkMap hash
	// lookup entirely and, same as before, does not bother updating the access
	// timestamp here -- a chunk only reached through this path is the one the
	// player is standing in, always inside the unload radius regardless of how
	// stale lastAccessTick is. Last-access tracking lives on the Chunk itself
	// now (see Chunk::lastAccessTick), not a side map keyed by chunk coordinate.
	if (lastChunk != nullptr && i == lastChunkX && j == lastChunkZ)
		return lastChunk;

	const std::uint64_t key = chunkKey(i, j);
	auto it = chunkMap.find(key);
	if (it == chunkMap.end())
	{
#if PLATFORM_BOUNDED_WORLD && PLATFORM_GENERATE_CHUNKS_PER_TICK > 0
		if (worldObj == nullptr || !worldObj->findingSpawnPoint)
		{
			long_t dcx = static_cast<long_t>(i) - static_cast<long_t>(curChunkX);
			long_t dcz = static_cast<long_t>(j) - static_cast<long_t>(curChunkZ);
			if (dcx < 0) dcx = -dcx;
			if (dcz < 0) dcz = -dcz;
			const long_t cheb = dcx > dcz ? dcx : dcz;
			const bool critical = cheb <= PLATFORM_GENERATE_SYNC_RADIUS;
#if PLATFORM_ASYNC_CHUNK_GENERATION
			// Non-critical terrain can be queued on a low-priority generation service.
			// Minecraft keeps rendering the current frame until the completed chunk is
			// published on a later tick.
			if (!critical && asyncGenerationScheduler != nullptr && asyncGenerationScheduler->active())
			{
				const ChunkRequestStatus requestStatus = requestChunkDetailed(i, j);
				if (requestStatus == ChunkRequestStatus::Accepted ||
					requestStatus == ChunkRequestStatus::AlreadyQueued)
					return blankChunk;
			}
#endif
			if (!critical && genChunksThisTick >= PLATFORM_GENERATE_CHUNKS_PER_TICK)
				return blankChunk;
			genChunksThisTick++;
		}
#endif
		return prepareChunk(i, j);
	}

	if (it->second != nullptr)
		it->second->lastAccessTick = currentWorldTime();
	if (it->second != nullptr && it->second != blankChunk)
	{
		lastChunk  = it->second;
		lastChunkX = i;
		lastChunkZ = j;
	}
	return it->second;
}

Chunk *ChunkProvider::loadChunkFromFile(int_t i, int_t j, bool &readFailed)
{
	readFailed = false;
	if (chunkLoader == nullptr) return nullptr;
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
		MC_LOG_ERROR("chunk", "ChunkProvider::loadChunkFromFile - exception loading %d,%d\n", i, j);
	}
	return nullptr;
}

void ChunkProvider::saveExtraChunkData(Chunk *chunk)
{
	if (chunkLoader == nullptr || chunk == nullptr || chunk == blankChunk) return;
	try
	{
		chunkLoader->saveExtraChunkData(worldObj, chunk);
	}
	catch (...)
	{
		MC_LOG_ERROR("chunk", "ChunkProvider::saveExtraChunkData - exception\n");
	}
}

void ChunkProvider::saveChunkToFile(Chunk *chunk)
{
	if (chunkLoader == nullptr || chunk == nullptr || chunk == blankChunk) return;
	try
	{
		chunk->lastSaveTime = worldObj->getWorldTime();
		chunkLoader->saveChunk(worldObj, chunk);
	}
	catch (...)
	{
		MC_LOG_ERROR("chunk", "ChunkProvider::saveChunkToFile - exception\n");
	}
}

void ChunkProvider::unloadChunk(std::uint64_t key, Chunk *chunk)
{
	(void)key; // no longer needed: last-access tracking moved onto Chunk itself
	if (chunk == nullptr || chunk == blankChunk)
		return;

	// Drop the single-entry cache if it points at the chunk being freed.
	if (lastChunk == chunk)
		lastChunk = nullptr;

#if PLATFORM_SAVE_RUNTIME_CHUNK_EDITS_ON_UNLOAD
	// PS2 serializes only gameplay-edited chunks here. Generated/lighting-only
	// dirtiness stays memory-only so walking does not create continuous writes.
	if (!chunk->neverSave && chunk->isRuntimeSaveRequired())
	{
#if PLATFORM_PROFILE_STREAMING
		const long_t unloadSaveStartNs = System::nanoTime();
#endif
		saveChunkToFile(chunk);
		chunk->isModified = false;
		chunk->clearRuntimeSaveRequired();
		saveExtraChunkData(chunk);
#if PLATFORM_PROFILE_STREAMING
		platformProfileUnloadSave(System::nanoTime() - unloadSaveStartNs);
#endif
	}
#elif !PLATFORM_CONSOLE_LOW
	// Unloading must not force a disk write for clean chunks. The v10 cache
	// optimization saved every chunk on eviction, which caused heavy IO spikes
	// while walking. Save only chunks that Java would consider dirty/stale.
	if (!chunk->neverSave && chunk->needsSaving(false))
	{
#if PLATFORM_PROFILE_STREAMING
		const long_t unloadSaveStartNs = System::nanoTime();
#endif
		saveChunkToFile(chunk);
		chunk->isModified = false;
		chunk->clearRuntimeSaveRequired();
		saveExtraChunkData(chunk);
#if PLATFORM_PROFILE_STREAMING
		platformProfileUnloadSave(System::nanoTime() - unloadSaveStartNs);
#endif
	}
#endif

	chunk->onChunkUnload();
	delete chunk;
}

void ChunkProvider::populate(IChunkProvider *ichunkprovider, int_t i, int_t j)
{
#if PLATFORM_BOUNDED_WORLD
	while (!populateDeferredStep(i, j))
	{
	}
#else
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
#endif
}

#if PLATFORM_BOUNDED_WORLD
bool ChunkProvider::populateDeferredStep(int_t i, int_t j)
{
	Chunk *chunk = provideChunk(i, j);
	if (chunk == nullptr || chunk == blankChunk || chunk->isTerrainPopulated)
		return true;
	if (chunkProvider == nullptr)
	{
		chunk->isTerrainPopulated = true;
		return true;
	}

	Chunk *populationChunks[POPULATION_FOOTPRINT_AXIS * POPULATION_FOOTPRINT_AXIS] = {};
	std::uint32_t before[POPULATION_FOOTPRINT_AXIS * POPULATION_FOOTPRINT_AXIS]
	                    [POPULATION_SECTION_COUNT] = {};
	for (int_t dz = 0; dz < POPULATION_FOOTPRINT_AXIS; ++dz)
	{
		for (int_t dx = 0; dx < POPULATION_FOOTPRINT_AXIS; ++dx)
		{
			const int_t footprintIndex = dz * POPULATION_FOOTPRINT_AXIS + dx;
			Chunk *footprintChunk = getLoadedChunk(JavaArithmetic::intAdd(i, dx), JavaArithmetic::intAdd(j, dz));
			populationChunks[footprintIndex] = footprintChunk;
			for (int_t sectionY = 0; sectionY < POPULATION_SECTION_COUNT; ++sectionY)
			{
				before[footprintIndex][sectionY] = footprintChunk != nullptr
					? footprintChunk->getBlockSectionRevision(sectionY)
					: 0u;
			}
		}
	}

	if (worldObj != nullptr)
		worldObj->beginPopulationFastPath(i, j);

	bool complete = false;
	try
	{
		complete = chunkProvider->populateStep(this, i, j);
	}
	catch (...)
	{
		if (worldObj != nullptr)
			worldObj->endPopulationFastPath();
		throw;
	}

	if (worldObj != nullptr)
		worldObj->endPopulationFastPath();

	if (worldObj != nullptr)
	{
		for (int_t footprintIndex = 0;
			 footprintIndex < POPULATION_FOOTPRINT_AXIS * POPULATION_FOOTPRINT_AXIS;
			 ++footprintIndex)
		{
			Chunk *footprintChunk = populationChunks[footprintIndex];
			if (footprintChunk == nullptr || footprintChunk == blankChunk)
				continue;

			for (int_t sectionY = 0; sectionY < POPULATION_SECTION_COUNT; ++sectionY)
			{
				if (before[footprintIndex][sectionY] ==
					footprintChunk->getBlockSectionRevision(sectionY))
					continue;

				const int_t minX = JavaArithmetic::intMul(footprintChunk->xPosition, 16);
				const int_t minY = sectionY << 4;
				const int_t minZ = JavaArithmetic::intMul(footprintChunk->zPosition, 16);
				worldObj->markBlocksDirty(minX, minY, minZ,
				                          JavaArithmetic::intAdd(minX, 15), minY + 15,
				                          JavaArithmetic::intAdd(minZ, 15));
			}
		}
	}

	if (complete)
	{
		chunk->isTerrainPopulated = true;
		chunk->setChunkModified();
	}
	return complete;
}

Chunk *ChunkProvider::getLoadedChunk(int_t i, int_t j)
{
	auto it = chunkMap.find(chunkKey(i, j));
	return it != chunkMap.end() ? it->second : nullptr;
}

bool ChunkProvider::canPopulateChunk(int_t i, int_t j)
{
	// Read straight from the map -- never call provideChunk() here, or the gate
	// would force-generate the very neighbours we are only meant to test for.
	Chunk *c = getLoadedChunk(i, j);
	if (c == nullptr || c == blankChunk || c->isTerrainPopulated)
		return false;
	// Decoration writes up to +8 blocks into the +x/+z neighbours, so they (and
	// the diagonal) must already exist. Same invariant the inline path enforced.
	const int_t eastX = JavaArithmetic::intAdd(i, 1);
	const int_t southZ = JavaArithmetic::intAdd(j, 1);
	return chunkExists(eastX, j)
		&& chunkExists(i, southZ)
		&& chunkExists(eastX, southZ);
}

void ChunkProvider::enqueuePopulate(int_t i, int_t j)
{
#if PLATFORM_POPULATE_CHUNKS_PER_TICK <= 0
	// Deferred decoration is disabled on this platform; do not let the queue grow
	// (drainPendingPopulate never runs, so entries would accumulate forever).
	(void)i; (void)j;
	return;
#else
	Chunk *c = getLoadedChunk(i, j);
	if (c == nullptr || c == blankChunk || c->isTerrainPopulated)
		return;
	const std::uint64_t key = chunkKey(i, j);
	if (populateQueued.insert(key).second)
		populateQueue.emplace_back(i, j);
#endif
}

void ChunkProvider::drainPendingPopulate(int_t budget)
{
	if (budget <= 0)
		return;
	// Bound the scan to the current queue length so dropping not-yet-ready entries
	// cannot spin. A dropped chunk is re-enqueued when its last missing neighbour
	// is generated (that prepareChunk enqueues this chunk as one of its four).
	int_t scan = (int_t)populateQueue.size();
	int_t steps = 0;
#if PLATFORM_POPULATE_BUDGET_US > 0
	const long_t budgetStartNs = System::nanoTime();
	const long_t budgetNs = (long_t)PLATFORM_POPULATE_BUDGET_US * 1000LL;
#endif
	while (steps < budget && scan-- > 0 && !populateQueue.empty())
	{
		const std::pair<int_t, int_t> coord = populateQueue.front();
		populateQueue.pop_front();
		const std::uint64_t key = chunkKey(coord.first, coord.second);
		populateQueued.erase(key);

		if (!canPopulateChunk(coord.first, coord.second))
			continue;

		const bool complete = populateDeferredStep(coord.first, coord.second);
		steps++;
		if (!complete)
		{
			populateQueued.insert(key);
			populateQueue.emplace_front(coord);
			scan++;
		}
#if PLATFORM_POPULATE_BUDGET_US > 0
		if (System::nanoTime() - budgetStartNs >= budgetNs)
			break;
#endif
		if (!complete)
			continue;
	}
}
#endif

bool ChunkProvider::saveChunks(bool flag, IProgressUpdate *iprogressupdate)
{
	// Chunk persistence is region-file backed. Runtime autosaves call this with
	// flag=false from inside World::tick(), so keep that path deliberately small:
	// compressing and writing a large dirty batch synchronously produces a visible
	// libfat stall. Full/menu saves still drain every dirty chunk and flush the
	// RegionFile cache through saveExtraData() below.
	int_t saved = 0;
	int_t totalToSave = 0;
	if (iprogressupdate != nullptr)
	{
		for (Chunk *chunk : chunkList)
		{
			if (chunk != nullptr && chunk != blankChunk && chunk->needsSaving(flag))
				totalToSave++;
		}
	}

	int_t progress = 0;
	for (Chunk *chunk : chunkList)
	{
		if (chunk == nullptr || chunk == blankChunk) continue;
		if (flag && !chunk->neverSave)
			saveExtraChunkData(chunk);
		if (!chunk->needsSaving(flag))
			continue;
		saveChunkToFile(chunk);
		chunk->isModified = false;
		chunk->clearRuntimeSaveRequired();
		++saved;

		// Java can let the threaded writer queue grow because the desktop JVM has
		// a large GC heap.  In this C++ port every queued chunk owns a complete NBT
		// tree until the writer consumes it.  A new-world full save can otherwise
		// retain hundreds of chunk NBT trees at once and exhaust/fragment the heap
		// while the loading screen says "Saving chunks".  Drain in bounded batches
		// without changing which chunks are saved or the on-disk format.
		if (flag && (saved % std::max<int_t>(1, PLATFORM_INCREMENTAL_CHUNK_SAVE_LIMIT)) == 0)
			ThreadedFileIOBase::threadedIOInstance.waitForFinish();

		// Release 1.2.5 uses 24 here. Platform tuning may lower the batch on
		// storage-constrained consoles without changing desktop/parity behavior.
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

bool ChunkProvider::unload100OldestChunks()
{
#if PLATFORM_BOUNDED_WORLD
	// Per-tick hook (World::tick calls this once). Refill the synchronous-generation
	// budget for the new tick, then decorate a small budget of queued chunks off the
	// generation frame. Independent of the unload return value, so the
	// unloadAllChunks() drain loop is unaffected.
	genChunksThisTick = 0;
#if PLATFORM_ASYNC_CHUNK_GENERATION
	const bool asyncPublishedChunk = drainAsyncGeneratedChunks(PLATFORM_ASYNC_GENERATION_PUBLISH_PER_TICK);
	// Dispatching does NOT stand the decoration budget down, publishing does.
	//
	// ChunkGenerationScheduler::dispatch() moves a coordinate from the pending
	// queue to the worker's queue under a mutex and returns; the generation
	// itself runs on runWiiGenerationThread(). Letting those microseconds close
	// this gate cost the whole 1800us decoration budget of every tick that
	// queued a request -- while the player explores, nearly all of them. The
	// decorator is one feature per step (see BiomeDecorator's stage machine), so
	// starving it does not slow decoration down, it stops it: chunks stayed
	// terrain-complete and undecorated for as long as the player kept walking.
	// Trees are what you notice, because they sit late in that order, after the
	// ores, clay and sand that are underground or unremarkable.
	//
	// Publishing is a real generation frame -- drainAsyncGeneratedChunks() is
	// what adds the ~80 KB column to chunkMap and regenerates its skylight -- so
	// that one still keeps decoration off the same tick.
	drainAsyncGenerationRequests(PLATFORM_ASYNC_GENERATION_REQUESTS_PER_TICK);
#else
	const bool asyncPublishedChunk = false;
#endif
	if (!asyncPublishedChunk)
	{
#if PLATFORM_PROFILE_STREAMING
		const long_t populateStartNs = System::nanoTime();
#endif
		drainPendingPopulate(PLATFORM_POPULATE_STEPS_PER_TICK);
#if PLATFORM_PROFILE_STREAMING
		platformProfilePopulate(System::nanoTime() - populateStartNs);
#endif
	}
#endif

#if PLATFORM_PROFILE_STREAMING
	const long_t chunkEvictStartNs = System::nanoTime();
#endif
	int_t unloaded = 0;

	// Old explicit drop queue, kept for compatibility with the decompiled layout.
	while (!droppedChunksSet.empty() && unloaded < MAX_UNLOADS_PER_TICK)
	{
		std::uint64_t key = *droppedChunksSet.begin();
		droppedChunksSet.erase(droppedChunksSet.begin());

		auto it = chunkMap.find(key);
		if (it == chunkMap.end())
			continue;

		Chunk *chunk = it->second;
		unloadChunk(key, chunk);
		chunkMap.erase(it);
		chunkList.erase(std::remove(chunkList.begin(), chunkList.end(), chunk), chunkList.end());
		unloaded++;
	}

	// Real distance-based unload: the previous port never inserted into
	// droppedChunksSet, so chunks stayed in memory forever. This keeps the visible
	// radius plus margin, and removes only chunks that are already outside it.
	//
	// EMERGENCY UNLOAD: if the resident chunk map has grown well past what the
	// unload radius should ever hold, ignore MAX_UNLOADS_PER_TICK and the time
	// gate and drain out-of-radius chunks at an accelerated bounded rate. Without this, a burst
	// that gets chunks into the map faster than 8/tick can drain them (e.g. a
	// stall or a hitch during initial load bunching several ticks' worth of
	// requests together) lets the map grow without bound until the heap is
	// exhausted -- this mirrors the equivalent fix in
	// ChunkProviderLoadOrGenerate::unload100OldestChunks; both cache tiers need
	// it since ChunkProvider is the one actually in the World::getChunkProvider()
	// path.
	const size_t maxResidentChunks = (size_t)((chunkUnloadRadius * 2 + 1) * (chunkUnloadRadius * 2 + 1));
	size_t chunksOutsideRadius = 0;
	for (const auto &entry : chunkMap)
	{
		Chunk *chunk = entry.second;
		if (chunk != nullptr && chunk != blankChunk
			&& isOutsideUnloadRadius(chunk->xPosition, chunk->zPosition))
		{
			chunksOutsideRadius++;
		}
	}
	const bool emergency = chunkMap.size() > maxResidentChunks && chunksOutsideRadius > 0;
	const int_t unloadLimit = emergency
        ? PLATFORM_EMERGENCY_CHUNK_UNLOADS_PER_TICK
        : MAX_UNLOADS_PER_TICK;

	const long_t now = currentWorldTime();
	for (auto it = chunkMap.begin(); it != chunkMap.end() && unloaded < unloadLimit; )
	{
		Chunk *chunk = it->second;
		if (chunk == nullptr || chunk == blankChunk)
		{
			it = chunkMap.erase(it);
			chunkList.erase(std::remove(chunkList.begin(), chunkList.end(), chunk), chunkList.end());
			continue;
		}

		const long_t lastAccess = chunk->lastAccessTick;

		if (isOutsideUnloadRadius(chunk->xPosition, chunk->zPosition)
			&& (emergency || JavaArithmetic::longSub(now, lastAccess) >= MIN_UNUSED_TICKS_BEFORE_UNLOAD))
		{
			unloadChunk(it->first, chunk);
			it = chunkMap.erase(it);
			chunkList.erase(std::remove(chunkList.begin(), chunkList.end(), chunk), chunkList.end());
			unloaded++;
			if (chunksOutsideRadius > 0)
				chunksOutsideRadius--;
			if (emergency && chunksOutsideRadius == 0)
				break;
		}
		else
		{
			++it;
		}
	}

#if PLATFORM_PROFILE_STREAMING
	platformProfileChunkEvict(System::nanoTime() - chunkEvictStartNs);
#endif
	if (chunkLoader != nullptr)
		chunkLoader->chunkTick();

	const bool childUnloaded = chunkProvider != nullptr && chunkProvider->unload100OldestChunks();
	return unloaded > 0 || childUnloaded;
}

bool ChunkProvider::canSave()
{
	return true;
}

jstring ChunkProvider::makeString()
{
	jstring result = "ServerChunkCache: " + String::fromInt((int_t)chunkMap.size())
		+ " Radius: " + String::fromInt(chunkLoadRadius)
		+ " UnloadRadius: " + String::fromInt(chunkUnloadRadius)
		+ " Drop: " + String::fromInt((int_t)droppedChunksSet.size());
#if PLATFORM_BOUNDED_WORLD && PLATFORM_ASYNC_CHUNK_GENERATION
	if (asyncGenerationScheduler != nullptr)
	{
		int_t pending = 0, completed = 0;
		asyncGenerationScheduler->queueSizes(pending, completed);
		result += " GenQ: " + String::fromInt(pending)
		       + " GenDone: " + String::fromInt(completed);
	}
#endif
	return result;
}

std::vector<SpawnListEntry> *ChunkProvider::getPossibleCreatures(const EnumCreatureType &type, int_t x, int_t y, int_t z)
{
	return chunkProvider != nullptr ? chunkProvider->getPossibleCreatures(type, x, y, z) : nullptr;
}

ChunkPosition *ChunkProvider::findClosestStructure(World *world, const jstring &name, int_t x, int_t y, int_t z)
{
	return chunkProvider != nullptr ? chunkProvider->findClosestStructure(world, name, x, y, z) : nullptr;
}

void ChunkProvider::removeEntityFromLoadedChunks(Entity *entity)
{
	for (const auto &entry : chunkMap)
	{
		Chunk *chunk = entry.second;
		if (chunk != nullptr && chunk != blankChunk)
			chunk->removeEntityFromAllSections(entity);
	}
}
