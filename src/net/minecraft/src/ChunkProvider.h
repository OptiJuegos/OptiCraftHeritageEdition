#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <deque>
#include <utility>

#include <cstdint>

#include "platform/PlatformTuning.h"

#include "IChunkProvider.h"
#include "java/Type.h"
#include "java/String.h"

class World;
class Chunk;
class IChunkLoader;
class IProgressUpdate;
class McRegionChunkLoader;
class AnvilChunkLoader;
class ChunkGenerationScheduler;

// net.minecraft.src.ChunkProvider
class ChunkProvider : public IChunkProvider
{
public:
	ChunkProvider(World *world, IChunkLoader *ichunkloader, IChunkProvider *ichunkprovider);
	~ChunkProvider() override;

	void setCurrentChunkOver(int_t i, int_t j);
	void setChunkLoadRadius(int_t radius);
	void setChunkLoadRadiusFromRenderDistance(int_t renderDistance);
#if PLATFORM_ASYNC_CHUNK_GENERATION
	enum class ChunkRequestStatus
	{
		Accepted,
		AlreadyQueued,
		AlreadyLoaded,
		QueueFull,
		OutOfRange,
		// Data exists on disk; the worker only generates, so the caller loads
		// it on the game thread instead.
		SavedOnDisk,
		Inactive
	};

	ChunkRequestStatus requestChunkDetailed(int_t i, int_t j);
	bool requestChunk(int_t i, int_t j);
	void serviceAsyncChunkStreaming();
#endif
	bool canChunkExist(int_t i, int_t j) const;
#if PLATFORM_INCREMENTAL_CHUNK_GENERATION
	bool isChunkGenerationPending(int_t i, int_t j) const;
	// Per-frame slice of the incremental generator, on top of the per-tick one
	// in unload100OldestChunks(). Frames run 2-3x more often than ticks, so the
	// same per-slice budget streams a chunk in a fraction of the wall time.
	// Called by EntityRenderer::renderWorld. See PLATFORM_GENERATION_STEPS_PER_FRAME.
	void serviceFrameGeneration();
#endif

	bool    chunkExists(int_t i, int_t j) override;
	Chunk  *getChunkIfExists(int_t i, int_t j) override;
	Chunk  *prepareChunk(int_t i, int_t j) override;
	Chunk  *provideChunk(int_t i, int_t j) override;
	void    populate(IChunkProvider *ichunkprovider, int_t i, int_t j) override;
	bool    saveChunks(bool flag, IProgressUpdate *iprogressupdate) override;
	bool    unload100OldestChunks() override;
	bool    canSave() override;
	jstring makeString() override;
	std::vector<SpawnListEntry> *getPossibleCreatures(const EnumCreatureType &type, int_t x, int_t y, int_t z) override;
	ChunkPosition *findClosestStructure(World *world, const jstring &name, int_t x, int_t y, int_t z) override;
	int_t   getLoadedChunkCount() const override { return (int_t)chunkMap.size(); }
	std::uint32_t getChunkTopologyVersion() const override { return chunkTopologyVersion; }
	bool isChunkPopulationPending(int_t i, int_t j) const override;
	std::vector<Chunk *> getLoadedChunksSnapshot() const override { return chunkList; }
	void    removeEntityFromLoadedChunks(Entity *entity) override;

private:
	static std::uint64_t chunkKey(int_t i, int_t j);

	Chunk *loadChunkFromFile(int_t i, int_t j, bool &readFailed);
	Chunk *prepareChunkInternal(int_t i, int_t j, bool deferGeneration);
	void   publishPreparedChunk(int_t i, int_t j, Chunk *chunk);
#if PLATFORM_INCREMENTAL_CHUNK_GENERATION
	void   enqueueGeneration(int_t i, int_t j);
	void   cancelQueuedGeneration(int_t i, int_t j);
	bool   drainPendingGeneration(int_t stepBudget, bool &publishedChunk);
	bool   drainPendingGeneration(int_t stepBudget, long_t budgetNs, bool &publishedChunk);
	int_t  countLoadedGenerationNeighbours(int_t i, int_t j) const;
	std::deque<std::pair<int_t, int_t>>::iterator selectNextGenerationCoordinate();
#endif
	void   saveExtraChunkData(Chunk *chunk);  // func_28063_a
	void   saveChunkToFile(Chunk *chunk);     // func_28062_b
	void   unloadChunk(std::uint64_t key, Chunk *chunk);
	long_t currentWorldTime() const;
	bool   isOutsideUnloadRadius(int_t i, int_t j) const;
	void   markChunkTopologyChanged();
	// chunkExists() for population gating: never-land (chunks outside a
	// limited world) counts as existing -- see the .cpp for why. Lives at
	// file scope (not under PLATFORM_DEFERRED_POPULATE) because the
	// synchronous populate path in publishPreparedChunk uses it too.
	bool   chunkExistsForPopulate(int_t i, int_t j);

#if PLATFORM_DEFERRED_POPULATE
	// Deferred decoration queue (see PLATFORM_POPULATE_CHUNKS_PER_TICK). prepareChunk
	// enqueues chunks whose +neighbours exist instead of populating inline, and the
	// per-tick drain decorates a small budget off the generation frame. This also
	// removes the populate->setBlock->provideChunk->populate re-entrancy.
	Chunk *getLoadedChunk(int_t i, int_t j);
	bool   canPopulateChunk(int_t i, int_t j);
	void   enqueuePopulate(int_t i, int_t j);
	void   drainPendingPopulate(int_t budget);
	bool   populateDeferredBatch(int_t i, int_t j, int_t maxSteps, long_t deadlineNs, int_t &stepsRun);
	bool   populateDeferredStep(int_t i, int_t j);

	std::deque<std::pair<int_t, int_t>> populateQueue;
	std::unordered_set<std::uint64_t>   populateQueued;
#endif

#if PLATFORM_BOUNDED_WORLD || PLATFORM_ASYNC_CHUNK_GENERATION
	void   notifyChunkPublished(Chunk *chunk);
#endif
#if PLATFORM_ASYNC_CHUNK_GENERATION
	static bool acceptAsyncGenerationCoordinate(void* context, int_t i, int_t j);
	bool drainAsyncGenerationRequests(int_t budget);
	bool drainAsyncGeneratedChunks(int_t budget);
	ChunkGenerationScheduler *asyncGenerationScheduler;
	// Set when the loader is Anvil: the worker then has no region reader, and
	// requestChunkDetailed() has to keep saved chunks off its queue itself.
	AnvilChunkLoader *asyncSavedChunkProbe;
#endif
#if PLATFORM_BOUNDED_WORLD
	// Per-tick synchronous-generation budget (see PLATFORM_GENERATE_CHUNKS_PER_TICK).
	// Reset at the start of every world tick; incremented for each chunk generated.
	int_t genChunksThisTick;
#endif

#if PLATFORM_INCREMENTAL_CHUNK_GENERATION
	std::deque<std::pair<int_t, int_t>> generationQueue;
	std::unordered_set<std::uint64_t>   generationQueued;
	int_t generationMoveX;
	int_t generationMoveZ;
	bool  generationCenterInitialized;
#endif

	std::unordered_set<std::uint64_t>       droppedChunksSet;
	Chunk                                *blankChunk;       // field_28064_b
	IChunkProvider                       *chunkProvider;
	IChunkLoader                         *chunkLoader;
	std::unordered_map<std::uint64_t, Chunk *> chunkMap;
	std::vector<Chunk *>                   chunkList;
	// Single-entry lookup cache (OptiFine-style): block access sweeps hit the same
	// chunk thousands of times in a row, so short-circuit before touching chunkMap.
	Chunk                                *lastChunk;
	int_t                                 lastChunkX;
	int_t                                 lastChunkZ;
	World                                *worldObj;         // field_28066_g
	int_t                                 curChunkX;
	int_t                                 curChunkZ;
	int_t                                 chunkLoadRadius;
	int_t                                 chunkUnloadRadius;
	std::uint32_t                         chunkTopologyVersion;
};
