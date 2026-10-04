#include "WorldClient.h"
#include "platform/Log.h"
#include "WorldSettings.h"
#include "WorldInfo.h"
#include "java/Arithmetic.h"

#include "Chunk.h"
#include "ChunkCoordinates.h"
#include "ChunkCoordIntPair.h"
#include "ChunkProviderClient.h"
#include "Entity.h"
#include "EntityArrow.h"
#include "EntityClientPlayerMP.h"
#include "EntityPlayer.h"
#include "IWorldAccess.h"
#include "MCHash.h"
#include "Minecraft.h"
#include "NetClientHandler.h"
#include "Packet255KickDisconnect.h"
#include "SaveHandlerMP.h"
#include "WorldBlockPositionType.h"
#include "WorldInfo.h"
#include "WorldProvider.h"
#include "WorldProviderSurface.h"
#include "WorldHeight.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>
#include <zlib.h>
#include "platform/PlatformTuning.h"

namespace
{
bool isActiveClientEntity(Entity *entity)
{
    Minecraft *minecraft = Minecraft::getMinecraft();
    return minecraft != nullptr &&
           (static_cast<void *>(minecraft->thePlayer) == static_cast<void *>(entity) ||
            static_cast<void *>(minecraft->renderViewEntity) == static_cast<void *>(entity));
}

long_t getChunkDistance(int_t chunkX, int_t chunkZ, int_t centerX, int_t centerZ)
{
    const long_t dx = std::abs(static_cast<long_t>(chunkX) - static_cast<long_t>(centerX));
    const long_t dz = std::abs(static_cast<long_t>(chunkZ) - static_cast<long_t>(centerZ));
    return std::max(dx, dz);
}

#if PLATFORM_MP_DEFERRED_CHUNKS
bool inflateDeferredChunkPacket(const std::vector<byte_t> &compressed, int_t primaryMask,
                                bool includeInitialize, std::vector<byte_t> &out)
{
    int_t sectionCount = 0;
    for (int_t section = 0; section < 16; ++section)
        sectionCount += (primaryMask >> section) & 1;

    const std::size_t outputSize = 12288u * static_cast<std::size_t>(sectionCount)
        + (includeInitialize ? 256u : 0u);
    if (compressed.empty() || outputSize == 0)
        return false;

    out.assign(outputSize, 0);
    uLongf actual = static_cast<uLongf>(out.size());
    return uncompress(reinterpret_cast<Bytef *>(out.data()), &actual,
                      reinterpret_cast<const Bytef *>(compressed.data()),
                      static_cast<uLong>(compressed.size())) == Z_OK;
}
#endif
}


WorldClient::WorldClient(NetClientHandler *queue, long_t seed, int_t dimension)
	: World(new SaveHandlerMP(), "MpServer", WorldProvider::getProviderForDimension(dimension), seed),
	  sendQueue(queue), clientChunkProvider(nullptr), entityHash(new MCHash())
{
	ChunkCoordinates spawn(8, 64, 8); // stack: setSpawnPoint only copies x/y/z, the Java ptr was GC'd
	setSpawnPoint(&spawn);
	setMapStorage(queue->getMapStorage());

	// World's ctor already ran chunkProvider = getChunkProvider(), but a virtual
	// call from the base ctor dispatches to World::getChunkProvider (the
	// WorldClient vtable isn't active yet), so it built a plain ChunkProvider and
	// left clientChunkProvider null -> doPreChunk crashed. Redo it here, where the
	// override resolves correctly, and discard the base provider. (Base-ctor
	// virtual gotcha.)
	delete chunkProvider;
	chunkProvider = getChunkProvider();
#if PLATFORM_MP_DEFERRED_CHUNKS
	// Fix the bucket array at the configured ceiling. Growing and rehashing this
	// map in the middle of the initial Packet51 burst needlessly fragments the
	// small console heap.
	deferredChunks.reserve(PLATFORM_MP_MAX_DEFERRED_CHUNKS);
#endif
}

WorldClient::WorldClient(NetClientHandler *queue, const WorldSettings &settings, int_t dimension, int_t difficulty)
	: WorldClient(queue, settings.getSeed(), dimension)
{
	difficultySetting = difficulty;
	if (worldInfo != nullptr)
		worldInfo->setTerrainType(settings.getTerrainType());
}

WorldClient::~WorldClient()
{
	for (WorldBlockPositionType *change : pendingBlockChanges)
		delete change;

	// Network entities waiting for a chunk are intentionally detached from
	// World::loadedEntityList. Reattach their ownership before the base World
	// destructor builds its deduplicated delete set, otherwise a disconnect
	// while a chunk is absent would leak those allocations.
	for (Entity *entity : knownEntities.valuesInIterationOrder())
	{
		if (entity != nullptr &&
		    std::find(loadedEntityList.begin(), loadedEntityList.end(), entity) == loadedEntityList.end())
		{
			loadedEntityList.push_back(entity);
			trackLoadedEntityPointer(entity);
		}
	}

	delete entityHash;
}

void WorldClient::tick()
{
	setWorldTime(JavaArithmetic::longAdd(getWorldTime(), 1LL));
#if PLATFORM_PS2
	// This override does not run World::tick(). Advance the client-only weather
	// interpolation once per tick, including expiration of lightning flashes.
	// Do not run the base world's server-side weather timers or simulation.
	updateWeather();
#endif
	int_t light = calculateSkylightSubtracted(1.0f);
	if (light != skylightSubtracted)
	{
		skylightSubtracted = light;
		for (IWorldAccess *access : worldAccesses) access->updateAllRenderers();
	}

	// Packet processing used to happen after the pending-entity retry. On PS2 a
	// spawn and its map chunk could therefore not be joined until a later world
	// tick, and the general terrain promotion order could delay that still more.
	// Keep the same bounded promotion/cache policy, but make received state ready
	// before retrying its entities. The bounded Wii/3DS clients take the same
	// order now: their promoteDeferredChunks() ran with a null entity list, so
	// the entity-priority lane of the non-PS2 branch (written for it, but never
	// reached) was dead code and a pending spawn's chunk competed only by
	// distance, waiting however many ticks the per-tick promotion budget
	// needed to reach it. Desktop keeps its historical retry-then-dispatch
	// order in the #else below.
#if PLATFORM_MP_DEFERRED_CHUNKS
	sendQueue->processReadPackets();
	std::vector<Entity *> spawnCandidates = entitySpawnQueue.valuesInIterationOrder();
#if PLATFORM_PS2
	selectClientEntityRetryBatch(spawnCandidates, entityRetryCursor, 24);
#else
	if (spawnCandidates.size() > 10)
		spawnCandidates.resize(10);
#endif
	promoteDeferredChunks(&spawnCandidates);
#else
	std::vector<Entity *> spawnCandidates = entitySpawnQueue.valuesInIterationOrder();
	if (spawnCandidates.size() > 10)
		spawnCandidates.resize(10);
#endif
#if PLATFORM_MP_BOUNDED_CHUNK_CACHE
	// Bounded-console clients evict every tick whichever way chunks arrive --
	// deferred promotion above, direct inflate in NetClientHandler -- or a
	// server pushing its own view distance grows the client chunk map past
	// the heap. Desktop compiles this out (its chunk map is unbounded).
	trimClientChunkCache();
#endif
#if PLATFORM_MP_BOUNDED_CHUNK_CACHE && !PLATFORM_MP_DEFERRED_CHUNKS
	// The trim above leaves holes the server will never repair (no chunk
	// request packet exists in this protocol). Replay the compressed payload
	// stash for columns the player is walking back into. Runs right after
	// the trim so a re-materialized column can't be evicted in the same tick.
	rematerializeStashedChunks();
#endif
	for (Entity *entity : spawnCandidates)
	{
		// Java removes iterator().next() before attempting the spawn. A failed
		// WorldClient::spawnEntityInWorld call then re-adds the entity, moving it
		// to the current HashSet bucket head and preserving the vanilla retry order.
		entitySpawnQueue.remove(entity);

		if (entity == nullptr || entity->isDead)
			continue;
		const bool loaded = std::find(loadedEntityList.begin(), loadedEntityList.end(), entity) != loadedEntityList.end();
		if (!loaded)
		{
			if (entityJoinedWorld(entity))
			{
#if PLATFORM_PS2
				MC_LOG_DEBUG("net.entity", "attached id=%d chunk=%d,%d pending=%zu\n",
					entity->entityId,
					MathHelper::floor_double(entity->posX / 16.0),
					MathHelper::floor_double(entity->posZ / 16.0),
					entitySpawnQueue.size());
#endif
			}
		}
		else if (!entity->addedToChunk)
		{
			const int_t chunkX = MathHelper::floor_double(entity->posX / 16.0);
			const int_t chunkZ = MathHelper::floor_double(entity->posZ / 16.0);
			if (chunkExists(chunkX, chunkZ))
				getChunkFromChunkCoords(chunkX, chunkZ)->addEntity(entity);
			else
				entitySpawnQueue.add(entity);
		}
	}

#if PLATFORM_WII
	Minecraft *minecraft = Minecraft::getMinecraft();
	if (minecraft != nullptr && minecraft->thePlayer != nullptr && minecraft->thePlayer->worldObj == this)
	{
		EntityClientPlayerMP *player = dynamic_cast<EntityClientPlayerMP *>(minecraft->thePlayer);
		if (player != nullptr)
			player->flushMotionUpdatesForWorldTick();
	}
#endif


#if !PLATFORM_MP_DEFERRED_CHUNKS
	// Desktop keeps its historical dispatch point: the pending-entity retry
	// above runs on last tick's queue, then the socket queue drains here.
	sendQueue->processReadPackets();
#endif
	for (auto it = pendingBlockChanges.begin(); it != pendingBlockChanges.end();)
	{
		WorldBlockPositionType *change = *it;
		if (--change->field_1206_d == 0)
		{
			World::setBlockAndMetadata(change->field_1202_a, change->field_1201_b, change->field_1207_c,
			                           change->field_1205_e, change->field_1204_f);
			World::markBlockNeedsUpdate(change->field_1202_a, change->field_1201_b, change->field_1207_c);
			delete change;
			it = pendingBlockChanges.erase(it);
		}
		else ++it;
	}

	if (clientChunkProvider != nullptr)
		clientChunkProvider->unload100OldestChunks();
	updateBlocksAndPlayCaveSounds();
#if PLATFORM_PS2 && PLATFORM_MP_DEFERRED_CHUNKS && MC_LOG_LEVEL >= 2
	// Sample the actual working set without materializing missing chunks.
	static unsigned int healthTicks = 0;
	if (++healthTicks >= 100 && clientChunkProvider != nullptr &&
		!playerEntities.empty() && playerEntities[0] != nullptr)
	{
		healthTicks = 0;
		const int_t cx = MathHelper::floor_double(playerEntities[0]->posX / 16.0);
		const int_t cz = MathHelper::floor_double(playerEntities[0]->posZ / 16.0);
		int resident = 0, pending = 0, missing = 0;
		for (int dz = -PLATFORM_CHUNK_CACHE_RADIUS; dz <= PLATFORM_CHUNK_CACHE_RADIUS; ++dz)
		for (int dx = -PLATFORM_CHUNK_CACHE_RADIUS; dx <= PLATFORM_CHUNK_CACHE_RADIUS; ++dx)
		{
			if (clientChunkProvider->hasChunk(cx + dx, cz + dz)) { ++resident; continue; }
			const auto entry = deferredChunks.find(ChunkCoordIntPair::chunkXZ2Long(cx + dx, cz + dz));
			if (entry != deferredChunks.end() && !entry->second.compressed.empty()) ++pending;
			else ++missing;
		}
		MC_LOG_DEBUG("net.chunk", "workingSet center=%d,%d resident=%d pending=%d missingBase=%d"
			" cache=%zu bytes=%zu evicted=%lu promoted=%lu corrupt=%lu packets=%lu unloads=%lu rain=%d strength=%.3f\n",
			(int)cx, (int)cz, resident, pending, missing, deferredChunks.size(), deferredChunkBytes,
			(unsigned long)deferredChunkEvictions, (unsigned long)deferredChunkPromotions,
			(unsigned long)deferredChunkCorruptions, sendQueue->getMapChunkCount(),
			sendQueue->getPreChunkUnloadCount(), worldInfo->getRaining() ? 1 : 0, (double)rainingStrength);
	}
#endif
}

void WorldClient::invalidateBlockReceiveRegion(int_t minX, int_t minY, int_t minZ,
	                                            int_t maxX, int_t maxY, int_t maxZ)
{
	for (auto it = pendingBlockChanges.begin(); it != pendingBlockChanges.end();)
	{
		WorldBlockPositionType *change = *it;
		if (change->field_1202_a >= minX && change->field_1201_b >= minY && change->field_1207_c >= minZ &&
		    change->field_1202_a <= maxX && change->field_1201_b <= maxY && change->field_1207_c <= maxZ)
		{
			delete change;
			it = pendingBlockChanges.erase(it);
		}
		else ++it;
	}
}

IChunkProvider *WorldClient::getChunkProvider()
{
	clientChunkProvider = new ChunkProviderClient(this);
	return clientChunkProvider;
}

void WorldClient::setSpawnLocation() { ChunkCoordinates spawn(8, 64, 8); setSpawnPoint(&spawn); }
void WorldClient::updateBlocksAndPlayCaveSounds()
{
	func_48461_r();
#if PLATFORM_CACHE_RANDOM_TICK_CHUNKS
	const std::vector<ChunkCoordIntPair> &tickOrder = positionsToUpdateOrder;
#else
	const std::vector<ChunkCoordIntPair> tickOrder = positionsToUpdate.valuesInIterationOrder();
#endif
	for (const ChunkCoordIntPair &pair : tickOrder)
	{
#if PLATFORM_BOUNDED_WORLD
		if (clientChunkProvider == nullptr || !clientChunkProvider->chunkExists(pair.chunkXPos, pair.chunkZPos))
			continue;
#endif
		Chunk *chunk = getChunkFromChunkCoords(pair.chunkXPos, pair.chunkZPos);
		func_48458_a(JavaArithmetic::intMul(pair.chunkXPos, 16), JavaArithmetic::intMul(pair.chunkZPos, 16), chunk);
	}
}
void WorldClient::scheduleBlockUpdate(int_t, int_t, int_t, int_t, int_t) {}
bool WorldClient::TickUpdates(bool) { return false; }

void WorldClient::doPreChunk(int_t chunkX, int_t chunkZ, bool load)
{
	if (load)
	{
#if PLATFORM_PS2 && PLATFORM_MP_DEFERRED_CHUNKS
		// Packet50 only announces that the server considers this column loaded.
		// Do not materialize an empty Chunk here: doing so makes the following
		// Packet51 look resident and forces its zlib inflate + section import to run
		// synchronously inside NetworkManager::processReadPackets(). A normal server
		// view-distance burst can otherwise spend most of a PS2 world tick decoding
		// terrain the renderer has not reached yet. Packet51 stays compressed in the
		// deferred cache and promoteDeferredChunks() creates the real column under the
		// bounded per-tick promotion budget.
		return;
#elif PLATFORM_MP_DEFERRED_CHUNKS
		// Other bounded clients keep their existing pre-chunk materialization policy.
		if (!shouldKeepChunk(chunkX, chunkZ))
			return;
		clientChunkProvider->prepareChunk(chunkX, chunkZ);
#else
		clientChunkProvider->prepareChunk(chunkX, chunkZ);
#endif
	}
	else
	{
		forgetDeferredChunk(chunkX, chunkZ);
#if PLATFORM_MP_BOUNDED_CHUNK_CACHE && !PLATFORM_MP_DEFERRED_CHUNKS
		// The server withdrew its watch on this column; it WILL resend it
		// on re-entry. A stale stash entry would shadow that fresh copy.
		forgetStashedChunk(chunkX, chunkZ);
#endif
		clientChunkProvider->unloadChunk(chunkX, chunkZ);
		markBlocksDirty(JavaArithmetic::intMul(chunkX, 16), 0, JavaArithmetic::intMul(chunkZ, 16), JavaArithmetic::intAdd(JavaArithmetic::intMul(chunkX, 16), 15), WorldHeight::HEIGHT, JavaArithmetic::intAdd(JavaArithmetic::intMul(chunkZ, 16), 15));
	}
}

void WorldClient::cacheCompressedChunk(int_t chunkX, int_t chunkZ, bool includeInitialize,
    int_t primaryMask, int_t addMask, std::vector<byte_t> compressed)
{
#if PLATFORM_MP_DEFERRED_CHUNKS
    constexpr std::size_t MAX_SECTION_UPDATES = 16;
    const ulong_t key = ChunkCoordIntPair::chunkXZ2Long(chunkX, chunkZ);
    auto existing = deferredChunks.find(key);
    // A delta without a retained initialize packet cannot reconstruct a chunk.
    // Do not create an unbounded collection of unusable placeholder entries.
    if (!includeInitialize && existing == deferredChunks.end())
        return;

    DeferredChunk &entry = deferredChunks[key];
    entry.chunkX = chunkX;
    entry.chunkZ = chunkZ;

    if (includeInitialize)
    {
        deferredChunkBytes -= entry.compressed.size();
        for (const DeferredMapUpdate &update : entry.sectionUpdates)
            deferredChunkBytes -= update.compressed.size();
        deferredBlockChangeCount -= entry.changes.size();

        entry.primaryMask = primaryMask;
        entry.addMask = addMask;
        entry.compressed = std::move(compressed);
        entry.sectionUpdates.clear();
        entry.changes.clear();
        deferredChunkBytes += entry.compressed.size();
    }
    else if (!entry.compressed.empty())
    {
        if (entry.sectionUpdates.size() >= MAX_SECTION_UPDATES)
        {
            deferredChunkBytes -= entry.sectionUpdates.front().compressed.size();
            entry.sectionUpdates.erase(entry.sectionUpdates.begin());
        }
        entry.sectionUpdates.push_back({primaryMask, addMask, std::move(compressed)});
        deferredChunkBytes += entry.sectionUpdates.back().compressed.size();
    }

    entry.stamp = ++deferredChunkStamp;
#if !PLATFORM_PS2
    // Keep the original per-packet accounting behavior on Wii.
    enforceDeferredChunkBudget();
#endif
#else
    (void)chunkX; (void)chunkZ; (void)includeInitialize;
    (void)primaryMask; (void)addMask; (void)compressed;
#endif
}

#if PLATFORM_PS2 && PLATFORM_MP_DEFERRED_CHUNKS
void WorldClient::finishDeferredChunkPacketBatch()
{
    enforceDeferredChunkBudget();
}
#endif

void WorldClient::deferBlockChange(int_t x, int_t y, int_t z, int_t blockId, int_t metadata)
{
#if PLATFORM_MP_DEFERRED_CHUNKS
    constexpr std::size_t MAX_DEFERRED_CHUNKS = PLATFORM_MP_MAX_DEFERRED_CHUNKS;
    constexpr std::size_t MAX_CHANGES_PER_CHUNK = PLATFORM_MP_MAX_CHANGES_PER_CHUNK;
    constexpr std::size_t MAX_DEFERRED_CHANGES = PLATFORM_MP_MAX_DEFERRED_CHANGES;
    const int_t chunkX = JavaArithmetic::intShr(x, 4);
    const int_t chunkZ = JavaArithmetic::intShr(z, 4);
    const ulong_t key = ChunkCoordIntPair::chunkXZ2Long(chunkX, chunkZ);
    auto existing = deferredChunks.find(key);
    if (existing == deferredChunks.end() && deferredChunks.size() >= MAX_DEFERRED_CHUNKS)
        return;
    DeferredChunk &entry = deferredChunks[key];
    entry.chunkX = chunkX;
    entry.chunkZ = chunkZ;
    for (DeferredBlockChange &change : entry.changes)
    {
        if (change.x == x && change.y == y && change.z == z)
        {
            change.blockId = blockId;
            change.metadata = metadata;
            entry.stamp = ++deferredChunkStamp;
            return;
        }
    }
    if (entry.changes.size() >= MAX_CHANGES_PER_CHUNK ||
        deferredBlockChangeCount >= MAX_DEFERRED_CHANGES)
        return;
    entry.changes.push_back({x, y, z, blockId, metadata});
    ++deferredBlockChangeCount;
    entry.stamp = ++deferredChunkStamp;
#else
    (void)x; (void)y; (void)z; (void)blockId; (void)metadata;
#endif
}

#if PLATFORM_MP_BOUNDED_CHUNK_CACHE && !PLATFORM_MP_DEFERRED_CHUNKS

// ---------------------------------------------------------------------------
// Compressed-payload stash for the 3DS evict-only profile.
//
// The beta-1.2.5 protocol has no client->server chunk request packet and a
// vanilla/CraftBukkit server forgets nothing it sent while the player stays
// inside its (server-size!) view window -- so any column dropped locally by
// trimClientChunkCache used to become a permanent hole when walking back.
// Holding the compressed Packet51 per trimmed column and re-inflating it on
// re-approach costs ~5-15 KB/chunk (vs a full resident Chunk and its
// meshes) and absolutely does not unbound the live chunk map: the eviction in
// trimClientChunkCache still owns which columns are materialized.
// ---------------------------------------------------------------------------

void WorldClient::stashChunkPacket(int_t chunkX, int_t chunkZ, bool includeInitialize,
                                   int_t primaryMask, int_t addMask, std::vector<byte_t> compressed)
{
	if (compressed.empty())
		return;
	const ulong_t key = ChunkCoordIntPair::chunkXZ2Long(chunkX, chunkZ);
	auto existing = stashedChunks.find(key);
	// A delta without a retained base can never re-materialize the column --
	// don't let those allocate unbounded dead weight either.
	if (!includeInitialize && existing == stashedChunks.end())
		return;
	StashedChunkPayload &entry = stashedChunks[key];
	entry.chunkX = chunkX;
	entry.chunkZ = chunkZ;
	if (includeInitialize)
	{
		// A fresh full column replaces everything interim (deltas/changes were
		// already folded into it server-side).
		stashedChunkBytes -= entry.compressed.size();
		for (const DeferredMapUpdate &update : entry.sectionUpdates)
			stashedChunkBytes -= update.compressed.size();
		stashedBlockChangeCount -= entry.changes.size();
		entry.primaryMask = primaryMask;
		entry.addMask = addMask;
		entry.compressed = std::move(compressed);
		entry.sectionUpdates.clear();
		entry.changes.clear();
		stashedChunkBytes += entry.compressed.size();
	}
	else
	{
		if (entry.sectionUpdates.size() >= 16)
		{
			stashedChunkBytes -= entry.sectionUpdates.front().compressed.size();
			entry.sectionUpdates.erase(entry.sectionUpdates.begin());
		}
		entry.sectionUpdates.push_back({primaryMask, addMask, std::move(compressed)});
		stashedChunkBytes += entry.sectionUpdates.back().compressed.size();
	}
	enforceStashBudget();
}

void WorldClient::stashBlockChange(int_t x, int_t y, int_t z, int_t blockId, int_t metadata)
{
	const ulong_t key = ChunkCoordIntPair::chunkXZ2Long(JavaArithmetic::intShr(x, 4),
	                                                    JavaArithmetic::intShr(z, 4));
	auto it = stashedChunks.find(key);
	if (it == stashedChunks.end())
		return; // no retained base -> nothing to apply the change against later
	StashedChunkPayload &entry = it->second;
	for (DeferredBlockChange &change : entry.changes)
	{
		if (change.x == x && change.y == y && change.z == z)
		{
			change.blockId = blockId;
			change.metadata = metadata;
			return;
		}
	}
	if (entry.changes.size() >= PLATFORM_MP_MAX_CHANGES_PER_CHUNK ||
	    stashedBlockChangeCount >= PLATFORM_MP_MAX_DEFERRED_CHANGES)
		return;
	entry.changes.push_back({x, y, z, blockId, metadata});
	++stashedBlockChangeCount;
}

void WorldClient::enforceStashBudget()
{
	if (playerEntities.empty() || playerEntities[0] == nullptr)
		return;
	const int_t centerX = JavaArithmetic::intShr(JavaArithmetic::doubleToInt(std::floor(playerEntities[0]->posX)), 4);
	const int_t centerZ = JavaArithmetic::intShr(JavaArithmetic::doubleToInt(std::floor(playerEntities[0]->posZ)), 4);
	while (stashedChunkBytes > PLATFORM_MP_COMPRESSED_CHUNK_CACHE_BYTES && !stashedChunks.empty())
	{
		// Evict the column farthest from the player first -- the same distance
		// rule trimClientChunkCache uses for live chunks. NOTE: 3MB of
		// compressed payload cannot always hold a vanilla distance-10 window
		// (441 columns) -- columns flushed past the budget show as holes until
		// the player exits the server's window and walks back (a fresh send).
		// (the wire packets already carry chunk coordinates; the payload
		// stores them so no long-key decoding helper is needed)
		auto farthest = stashedChunks.begin();
		long_t farthestDist = -1;
		for (auto it = stashedChunks.begin(); it != stashedChunks.end(); ++it)
		{
			const long_t dist = getChunkDistance(it->second.chunkX, it->second.chunkZ, centerX, centerZ);
			if (dist > farthestDist)
			{
				farthestDist = dist;
				farthest = it;
			}
		}
		stashedChunkBytes -= farthest->second.compressed.size();
		for (const DeferredMapUpdate &update : farthest->second.sectionUpdates)
			stashedChunkBytes -= update.compressed.size();
		stashedBlockChangeCount -= farthest->second.changes.size();
		stashedChunks.erase(farthest);
		++stashedChunkEvictions;
	}
}

void WorldClient::forgetStashedChunk(int_t chunkX, int_t chunkZ)
{
	const ulong_t key = ChunkCoordIntPair::chunkXZ2Long(chunkX, chunkZ);
	auto it = stashedChunks.find(key);
	if (it == stashedChunks.end())
		return;
	stashedChunkBytes -= it->second.compressed.size();
	for (const DeferredMapUpdate &update : it->second.sectionUpdates)
		stashedChunkBytes -= update.compressed.size();
	stashedBlockChangeCount -= it->second.changes.size();
	stashedChunks.erase(it);
}

void WorldClient::rematerializeStashedChunks()
{
	if (playerEntities.empty() || playerEntities[0] == nullptr || clientChunkProvider == nullptr)
		return;
	const EntityPlayer *player = playerEntities[0];
	const int_t centerX = JavaArithmetic::intShr(JavaArithmetic::doubleToInt(std::floor(player->posX)), 4);
	const int_t centerZ = JavaArithmetic::intShr(JavaArithmetic::doubleToInt(std::floor(player->posZ)), 4);

	// The trim radius, not the renderer's: a column that just re-entered the
	// keep window must be resident BEFORE the renderer reaches it, otherwise
	// it re-meshes as air and reads as the permanent hole. Budget the zlib +
	// import work so a full perimeter ring doesn't stall a tick.
	constexpr int_t MAX_REMATERIALIZATIONS_PER_TICK = 4;
	int_t remaining = MAX_REMATERIALIZATIONS_PER_TICK;
	std::vector<ulong_t> toReimport;
	toReimport.reserve(stashedChunks.size());
	for (const auto &pair : stashedChunks)
	{
		const int_t cx = pair.second.chunkX;
		const int_t cz = pair.second.chunkZ;
		if (getChunkDistance(cx, cz, centerX, centerZ) <= PLATFORM_CHUNK_UNLOAD_RADIUS - 1 &&
		    !clientChunkProvider->hasChunk(cx, cz))
			toReimport.push_back(pair.first);
		// Columns back inside the LIVE radius get re-imported before the
		// renderer can reach them; the -1 keeps one ring of hysteresis so the
		// stash can't thrash against the trim at the exact boundary.
	}
	// Nearest first, so the column the player is walking towards wins the tick.
	std::sort(toReimport.begin(), toReimport.end(), [&](ulong_t a, ulong_t b)
	{
		const StashedChunkPayload &sa = stashedChunks.at(a);
		const StashedChunkPayload &sb = stashedChunks.at(b);
		return getChunkDistance(sa.chunkX, sa.chunkZ, centerX, centerZ) <
		       getChunkDistance(sb.chunkX, sb.chunkZ, centerX, centerZ);
	});

	remaining = MAX_REMATERIALIZATIONS_PER_TICK;
	for (ulong_t key : toReimport)
	{
		if (remaining <= 0)
			break;
		auto it = stashedChunks.find(key);
		if (it == stashedChunks.end())
			continue;
		StashedChunkPayload &entry = it->second;
		const int_t cx = entry.chunkX;
		const int_t cz = entry.chunkZ;

		Chunk *baseChunk = clientChunkProvider->prepareChunk(cx, cz);
		if (baseChunk == nullptr)
			break; // heap pressure: try again next tick instead of losing the payload

		// Same import discipline as NetClientHandler::handleMapChunk's direct
		// path: inflate with the worst-case section size, import through the
		// mask pair. Then replay the live-period section deltas and block
		// changes in arrival order -- they were cached after the base.
		bool ok = true;
		auto importSection = [&](int_t primaryMask, int_t addMask,
		                         const std::vector<byte_t> &payload, bool initialize) -> bool
		{
			// getChunkFromChunkCoords is a World method; the column is freshly
			// created above via prepareChunk so it always resolves here.
			Chunk *chunk = getChunkFromChunkCoords(cx, cz);
			if (chunk == nullptr)
				return false;
			std::vector<byte_t> inflated;
			int_t sections = 0;
			for (int_t s = 0; s < 16; ++s)
				sections += (primaryMask >> s) & 1;
			inflated.assign((std::size_t)sections * 12288u + (initialize ? 256u : 0u), 0);
			uLongf actual = (uLongf)inflated.size();
			if (uncompress(reinterpret_cast<Bytef *>(inflated.data()), &actual,
			               reinterpret_cast<const Bytef *>(payload.data()),
			               (uLong)payload.size()) != Z_OK)
				return false;
			inflated.resize(actual);
			return chunk->func_48494_a(inflated.data(), inflated.size(), primaryMask, addMask,
			                           initialize);
		};

		ok = importSection(entry.primaryMask, entry.addMask, entry.compressed, true);
		if (ok)
		{
			for (const DeferredMapUpdate &update : entry.sectionUpdates)
			{
				if (!importSection(update.primaryMask, update.addMask, update.compressed, false))
				{
					ok = false;
					break;
				}
			}
		}
		if (ok)
		{
			for (const DeferredBlockChange &change : entry.changes)
				baseChunk->setBlockIDWithMetadata(change.x, change.y, change.z,
				                                  change.blockId, change.metadata);
		}
		if (!ok)
		{
			// Corrupt payload: drop the chunk we made AND the stash entry --
			// half-state would render worse than a hole.
			clientChunkProvider->unloadChunk(cx, cz);
			forgetStashedChunk(cx, cz);
			continue;
		}
		if (dynamic_cast<WorldProviderSurface *>(worldProvider) == nullptr)
			baseChunk->resetRelightChecks();
		// Full-column dirty: the renderer's cells for an evicted column were
		// re-meshed as air (or never meshed) -- exactly like a fresh send.
		markBlocksDirty(JavaArithmetic::intShl(cx, 4), 0, JavaArithmetic::intShl(cz, 4),
		                JavaArithmetic::intAdd(JavaArithmetic::intShl(cx, 4), 15), WorldHeight::HEIGHT,
		                JavaArithmetic::intAdd(JavaArithmetic::intShl(cz, 4), 15));

		// KEEP the stash entry after a successful re-import: the trim will
		// evict the live column again on the next walk-away, and only this
		// payload can rebuild it (the server never resends inside its window).
		// Live-period deltas keep mirroring in, so it stays in sync.
		--remaining;
	}
}

// A column the live map evicted must remain re-materializable; but once the
// client KNOWS the server withdrew it (handlePreChunk mode=false), a stale
// stash must not shadow the fresh copy the server will send on re-entry. So
// forgetStashedChunk runs there (NetClientHandler::handlePreChunk), while the
// trim path deliberately keeps the payload.

#endif // PLATFORM_MP_BOUNDED_CHUNK_CACHE && !PLATFORM_MP_DEFERRED_CHUNKS


void WorldClient::forgetDeferredChunk(int_t chunkX, int_t chunkZ)
{
#if PLATFORM_MP_DEFERRED_CHUNKS
    const ulong_t key = ChunkCoordIntPair::chunkXZ2Long(chunkX, chunkZ);
    auto it = deferredChunks.find(key);
    if (it == deferredChunks.end())
        return;
    deferredChunkBytes -= it->second.compressed.size();
    for (const DeferredMapUpdate &update : it->second.sectionUpdates)
        deferredChunkBytes -= update.compressed.size();
    deferredBlockChangeCount -= it->second.changes.size();
    deferredChunks.erase(it);
#if !PLATFORM_PS2
    // Preserve the original deferred-cache accounting behavior off PS2.
    enforceDeferredChunkBudget();
#endif
#else
    (void)chunkX; (void)chunkZ;
#endif
}

void WorldClient::enforceDeferredChunkBudget()
{
#if PLATFORM_MP_DEFERRED_CHUNKS
    constexpr std::size_t MAX_BYTES = PLATFORM_MP_COMPRESSED_CHUNK_CACHE_BYTES;
#if PLATFORM_PS2
    constexpr std::size_t MAX_CHUNKS = PLATFORM_MP_MAX_DEFERRED_CHUNKS;
    auto overLimit = [&]()
    {
        return deferredChunkBytes > MAX_BYTES || deferredChunks.size() > MAX_CHUNKS;
    };

    const bool startedOverLimit = overLimit();
    if (startedOverLimit && !deferredChunkBudgetExceeded)
        ++deferredChunkBudgetOverflows;
    deferredChunkBudgetExceeded = startedOverLimit;
    if (!startedOverLimit)
        return;

    int_t centerX = 0;
    int_t centerZ = 0;
    const bool havePlayer = !playerEntities.empty() && playerEntities[0] != nullptr;
    if (havePlayer)
    {
        centerX = JavaArithmetic::intShr(MathHelper::floor_double(playerEntities[0]->posX), 4);
        centerZ = JavaArithmetic::intShr(MathHelper::floor_double(playerEntities[0]->posZ), 4);
    }

    struct EvictionCandidate
    {
        ulong_t key;
        bool hasBase;
        bool inWorkingSet;
        bool resident;
        long_t distance;
        ulong_t stamp;
    };

    std::vector<EvictionCandidate> victims;
    victims.reserve(deferredChunks.size());
    for (const auto &pair : deferredChunks)
    {
        const DeferredChunk &candidate = pair.second;
        const long_t distance = havePlayer
            ? getChunkDistance(candidate.chunkX, candidate.chunkZ, centerX, centerZ)
            : 0;
        victims.push_back({
            pair.first,
            !candidate.compressed.empty(),
            havePlayer && distance <= static_cast<long_t>(PLATFORM_CHUNK_UNLOAD_RADIUS),
            clientChunkProvider != nullptr && clientChunkProvider->hasChunk(candidate.chunkX, candidate.chunkZ),
            distance,
            candidate.stamp
        });
    }

    // Build the victim order once per network dispatch. The old path rescanned
    // the whole unordered_map for every single eviction; a view-distance 10 burst
    // could therefore turn cache pressure into quadratic EE work even though none
    // of those distant chunks was being inflated or rendered.
    std::sort(victims.begin(), victims.end(), [](const EvictionCandidate &a, const EvictionCandidate &b)
    {
        if (a.hasBase != b.hasBase)
            return !a.hasBase;
        if (a.inWorkingSet != b.inWorkingSet)
            return !a.inWorkingSet;
        if (a.distance != b.distance)
            return a.distance > b.distance;
        if (a.resident != b.resident)
            return a.resident;
        return a.stamp < b.stamp;
    });

    for (const EvictionCandidate &victim : victims)
    {
        if (!overLimit())
            break;
        auto it = deferredChunks.find(victim.key);
        if (it == deferredChunks.end())
            continue;
        deferredChunkBytes -= it->second.compressed.size();
        for (const DeferredMapUpdate &update : it->second.sectionUpdates)
            deferredChunkBytes -= update.compressed.size();
        deferredBlockChangeCount -= it->second.changes.size();
        deferredChunks.erase(it);
        ++deferredChunkEvictions;
    }

    deferredChunkBudgetExceeded = overLimit();
#else
    // Wii's budget is a soft reporting threshold. The server still owns these
    // columns and the protocol cannot request them again after local eviction.
    const bool overBudget = deferredChunkBytes > MAX_BYTES;
    if (overBudget && !deferredChunkBudgetExceeded)
        ++deferredChunkBudgetOverflows;
    deferredChunkBudgetExceeded = overBudget;
#endif
#endif
}

std::size_t WorldClient::getDeferredPromotionPendingCount() const
{
#if PLATFORM_MP_DEFERRED_CHUNKS
    if (playerEntities.empty() || playerEntities[0] == nullptr || clientChunkProvider == nullptr)
        return 0;
    std::size_t count = 0;
    for (const auto &pair : deferredChunks)
    {
        const DeferredChunk &entry = pair.second;
        if (!entry.compressed.empty() && shouldKeepChunk(entry.chunkX, entry.chunkZ) &&
            !clientChunkProvider->hasChunk(entry.chunkX, entry.chunkZ))
            ++count;
    }
    return count;
#else
    return 0;
#endif
}

void WorldClient::prioritizePlayerChunk(int_t chunkX, int_t chunkZ)
{
#if PLATFORM_MP_DEFERRED_CHUNKS
    if (clientChunkProvider == nullptr || clientChunkProvider->hasChunk(chunkX, chunkZ))
        return;
    promoteDeferredChunk(chunkX, chunkZ);
#else
    (void)chunkX;
    (void)chunkZ;
#endif
}

bool WorldClient::promoteDeferredChunk(int_t chunkX, int_t chunkZ)
{
#if PLATFORM_MP_DEFERRED_CHUNKS
    if (clientChunkProvider == nullptr || clientChunkProvider->hasChunk(chunkX, chunkZ))
        return false;

    const ulong_t key = ChunkCoordIntPair::chunkXZ2Long(chunkX, chunkZ);
    auto it = deferredChunks.find(key);
    if (it == deferredChunks.end() || it->second.compressed.empty())
        return false;

    DeferredChunk &entry = it->second;
    std::vector<byte_t> data;
    if (!inflateDeferredChunkPacket(entry.compressed, entry.primaryMask, true, data))
    {
        forgetDeferredChunk(entry.chunkX, entry.chunkZ);
        ++deferredChunkCorruptions;
        return false;
    }

    Chunk *chunk = clientChunkProvider->prepareChunk(entry.chunkX, entry.chunkZ);
    if (chunk == nullptr || !chunk->func_48494_a(data.data(), data.size(),
                                                 entry.primaryMask, entry.addMask, true))
    {
        forgetDeferredChunk(entry.chunkX, entry.chunkZ);
        ++deferredChunkCorruptions;
        return false;
    }

    for (const DeferredMapUpdate &update : entry.sectionUpdates)
    {
        if (!inflateDeferredChunkPacket(update.compressed, update.primaryMask, false, data) ||
            !chunk->func_48494_a(data.data(), data.size(), update.primaryMask, update.addMask, false))
        {
            forgetDeferredChunk(entry.chunkX, entry.chunkZ);
            ++deferredChunkCorruptions;
            return false;
        }
    }

    const int_t minX = JavaArithmetic::intShl(entry.chunkX, 4);
    const int_t minZ = JavaArithmetic::intShl(entry.chunkZ, 4);
    const int_t maxX = JavaArithmetic::intAdd(minX, 15);
    const int_t maxZ = JavaArithmetic::intAdd(minZ, 15);
    invalidateBlockReceiveRegion(minX, 0, minZ, maxX, WorldHeight::HEIGHT, maxZ);
#if PLATFORM_PS2
    // RenderGlobal expands dirty ranges by one block. Dirtying the complete
    // 16x16 column therefore also queues all eight neighbouring chunk columns,
    // multiplying the multiplayer mesh backlog even though only this column was
    // imported. Use the same interior range as ChunkProvider::notifyChunkPublished
    // so the expansion lands exactly on this chunk's bounds.
    markBlocksDirty(minX + 1, 1, minZ + 1, maxX - 1, WorldHeight::HEIGHT - 2, maxZ - 1);
    notifyChunkPublishedForRender(entry.chunkX, entry.chunkZ);
#else
    markBlocksDirty(minX, 0, minZ, maxX, WorldHeight::HEIGHT, maxZ);
#endif
    for (const DeferredBlockChange &change : entry.changes)
        setBlockAndMetadataAndInvalidate(change.x, change.y, change.z,
                                         change.blockId, change.metadata);
    entry.stamp = ++deferredChunkStamp;
    ++deferredChunkPromotions;
    return true;
#else
    (void)chunkX;
    (void)chunkZ;
    return false;
#endif
}

void WorldClient::promoteDeferredChunks(const std::vector<Entity *> *priorityEntities)
{
#if PLATFORM_MP_DEFERRED_CHUNKS
    if (playerEntities.empty() || playerEntities[0] == nullptr || clientChunkProvider == nullptr)
        return;
#if PLATFORM_PS2
    EntityPlayer *player = playerEntities[0];
    const int_t centerX = JavaArithmetic::intShr(MathHelper::floor_double(player->posX), 4);
    const int_t centerZ = JavaArithmetic::intShr(MathHelper::floor_double(player->posZ), 4);
    const double movementX = player->motionX;
    const double movementZ = player->motionZ;

    auto queuedEntityInChunk = [&](int_t chunkX, int_t chunkZ) -> Entity *
    {
        if (priorityEntities == nullptr)
            return nullptr;
        for (Entity *entity : *priorityEntities)
        {
            if (entity == nullptr || entity->isDead)
                continue;
            if (MathHelper::floor_double(entity->posX / 16.0) == chunkX &&
                MathHelper::floor_double(entity->posZ / 16.0) == chunkZ)
                return entity;
        }
        return nullptr;
    };

    for (int_t promoted = 0; promoted < PLATFORM_MP_CHUNK_PROMOTIONS_PER_TICK; ++promoted)
    {
        auto best = deferredChunks.end();
        long_t bestDistance = std::numeric_limits<long_t>::max();
        double bestAhead = -std::numeric_limits<double>::max();
        bool bestHasEntity = false;
        ulong_t bestStamp = std::numeric_limits<ulong_t>::max();
        Entity *bestPriorityEntity = nullptr;

        for (auto it = deferredChunks.begin(); it != deferredChunks.end(); ++it)
        {
            const DeferredChunk &candidate = it->second;
            if (candidate.compressed.empty() ||
                !shouldKeepChunk(candidate.chunkX, candidate.chunkZ) ||
                clientChunkProvider->hasChunk(candidate.chunkX, candidate.chunkZ))
                continue;

            const long_t distance = getChunkDistance(candidate.chunkX, candidate.chunkZ, centerX, centerZ);
            const double ahead =
                static_cast<double>(candidate.chunkX - centerX) * movementX +
                static_cast<double>(candidate.chunkZ - centerZ) * movementZ;
            Entity *priorityEntity = queuedEntityInChunk(candidate.chunkX, candidate.chunkZ);
            const bool hasEntity = priorityEntity != nullptr;

            // Terrain closest to the player is authoritative for promotion order.
            // Pending remote entities may break ties inside the same distance ring,
            // but must not consume a slot while a nearer terrain column is missing.
            // Within a ring, bias toward the direction of travel so the next border
            // is resident before the player reaches it.
            const bool better = best == deferredChunks.end() ||
                distance < bestDistance ||
                (distance == bestDistance && ahead > bestAhead) ||
                (distance == bestDistance && ahead == bestAhead && hasEntity != bestHasEntity && hasEntity) ||
                (distance == bestDistance && ahead == bestAhead && hasEntity == bestHasEntity &&
                 candidate.stamp < bestStamp);
            if (!better)
                continue;

            best = it;
            bestDistance = distance;
            bestAhead = ahead;
            bestHasEntity = hasEntity;
            bestStamp = candidate.stamp;
            bestPriorityEntity = priorityEntity;
        }

        if (best == deferredChunks.end())
            break;

        const int_t chunkX = best->second.chunkX;
        const int_t chunkZ = best->second.chunkZ;
        const bool promotedChunk = promoteDeferredChunk(chunkX, chunkZ);
        if (promotedChunk && bestPriorityEntity != nullptr)
        {
            ++deferredEntityChunkPromotions;
            MC_LOG_DEBUG("net.entity", "prioritized id=%d chunk=%d,%d pending=%zu\n",
                bestPriorityEntity->entityId, chunkX, chunkZ, entitySpawnQueue.size());
        }
        if (!promotedChunk &&
            deferredChunks.find(ChunkCoordIntPair::chunkXZ2Long(chunkX, chunkZ)) != deferredChunks.end())
            break;
    }
#else
    const int_t centerX = JavaArithmetic::intShr(MathHelper::floor_double(playerEntities[0]->posX), 4);
    const int_t centerZ = JavaArithmetic::intShr(MathHelper::floor_double(playerEntities[0]->posZ), 4);

    for (int_t promoted = 0; promoted < PLATFORM_MP_CHUNK_PROMOTIONS_PER_TICK; ++promoted)
    {
        auto best = deferredChunks.end();
        long_t bestDistance = std::numeric_limits<long_t>::max();
        ulong_t bestStamp = std::numeric_limits<ulong_t>::max();
		Entity *priorityEntity = nullptr;

		// A queued entity cannot enter loadedEntityList (and RenderGlobal cannot
		// draw it) until its real chunk replaces the shared EmptyChunk. Prefer its
		// resident-window chunk while consuming the same bounded promotion slot.
		if (priorityEntities != nullptr)
		{
		for (Entity *entity : *priorityEntities)
		{
			if (entity == nullptr || entity->isDead)
				continue;
			const int_t entityChunkX = MathHelper::floor_double(entity->posX / 16.0);
			const int_t entityChunkZ = MathHelper::floor_double(entity->posZ / 16.0);
			if (!shouldKeepChunk(entityChunkX, entityChunkZ) || clientChunkProvider->hasChunk(entityChunkX, entityChunkZ))
				continue;
			auto candidate = deferredChunks.find(ChunkCoordIntPair::chunkXZ2Long(entityChunkX, entityChunkZ));
			if (candidate == deferredChunks.end() || candidate->second.compressed.empty())
				continue;
			const long_t distance = getChunkDistance(entityChunkX, entityChunkZ, centerX, centerZ);
			if (best == deferredChunks.end() || distance < bestDistance ||
				(distance == bestDistance && candidate->second.stamp < bestStamp))
			{
				best = candidate;
				bestDistance = distance;
				bestStamp = candidate->second.stamp;
				priorityEntity = entity;
			}
		}
		}

		if (priorityEntity == nullptr)
		{
        for (auto it = deferredChunks.begin(); it != deferredChunks.end(); ++it)
        {
            const DeferredChunk &candidate = it->second;
            if (candidate.compressed.empty() ||
                !shouldKeepChunk(candidate.chunkX, candidate.chunkZ) ||
                clientChunkProvider->hasChunk(candidate.chunkX, candidate.chunkZ))
                continue;
            const long_t distance = getChunkDistance(candidate.chunkX, candidate.chunkZ, centerX, centerZ);
            if (distance < bestDistance ||
                (distance == bestDistance && candidate.stamp < bestStamp))
            {
                best = it;
                bestDistance = distance;
                bestStamp = candidate.stamp;
            }
        }
		}
        if (best == deferredChunks.end())
            break;

        const int_t chunkX = best->second.chunkX;
        const int_t chunkZ = best->second.chunkZ;
		const bool promotedChunk = promoteDeferredChunk(chunkX, chunkZ);
		if (promotedChunk && priorityEntity != nullptr)
		{
			++deferredEntityChunkPromotions;
			MC_LOG_DEBUG("net.entity", "prioritized id=%d chunk=%d,%d pending=%zu\n",
				priorityEntity->entityId, chunkX, chunkZ, entitySpawnQueue.size());
		}
        if (!promotedChunk &&
            deferredChunks.find(ChunkCoordIntPair::chunkXZ2Long(chunkX, chunkZ)) != deferredChunks.end())
            break;
    }
#endif
#endif
}

bool WorldClient::shouldKeepChunk(int_t chunkX, int_t chunkZ) const
{
#if PLATFORM_MP_DEFERRED_CHUNKS
	if (playerEntities.empty() || playerEntities[0] == nullptr)
		return false;
	const EntityPlayer *player = playerEntities[0];
	const int_t centerX = JavaArithmetic::intShr(JavaArithmetic::doubleToInt(std::floor(player->posX)), 4);
	const int_t centerZ = JavaArithmetic::intShr(JavaArithmetic::doubleToInt(std::floor(player->posZ)), 4);
#if PLATFORM_PS2
	// The active promotion window is the cache radius. The unload radius is only
	// hysteresis for columns that were already resident before the player moved;
	// using it here expands a nominal 5x5 PS2 working set into 7x7 in multiplayer.
	return getChunkDistance(chunkX, chunkZ, centerX, centerZ) <= PLATFORM_CHUNK_CACHE_RADIUS;
#else
	return getChunkDistance(chunkX, chunkZ, centerX, centerZ) <= PLATFORM_CHUNK_UNLOAD_RADIUS;
#endif
#else
	(void)chunkX;
	(void)chunkZ;
	return true;
#endif
}

void WorldClient::trimClientChunkCache()
{
#if PLATFORM_MP_BOUNDED_CHUNK_CACHE
	// Bounded-cache consoles (PS2, Wii, 3DS -- the PLATFORM_MP_BOUNDED_CHUNK_
	// CACHE table) evict live columns beyond PLATFORM_CHUNK_UNLOAD_RADIUS
	// (the same view+1 ring singleplayer keeps), whichever profile fills the
	// map: with PLATFORM_MP_DEFERRED_CHUNKS the promotion lane above, and
	// without it NetClientHandler's direct inflate. A server streams chunks
	// for ITS view distance, not the client's, so without this a vanilla
	// server at the default distance pushes ~441 columns the console cannot
	// hold -- the client chunk map grew without bound and surfaced as the
	// intermittent std::bad_alloc mid-session, and as the respawn burst
	// tipping an already-full heap. With the deferred cache compiled in,
	// the walk back re-inflates from the parked compressed copy; without it
	// (the evict-only profile) the column re-downloads from the server.
	if (playerEntities.empty() || playerEntities[0] == nullptr || clientChunkProvider == nullptr)
		return;
	const EntityPlayer *player = playerEntities[0];
	const int_t centerX = JavaArithmetic::intShr(JavaArithmetic::doubleToInt(std::floor(player->posX)), 4);
	const int_t centerZ = JavaArithmetic::intShr(JavaArithmetic::doubleToInt(std::floor(player->posZ)), 4);
	clientChunkProvider->unloadOutsideRadius(centerX, centerZ, PLATFORM_CHUNK_UNLOAD_RADIUS);
#endif
}

void WorldClient::applyNetworkPosition(Entity *entity, double x, double y, double z, float yaw, float pitch)
{
#if PLATFORM_PS2
	// Interpolation only runs for attached, ticking entities. A detached object
	// otherwise keeps its old pos forever while serverPos continues to advance.
	const int_t oldX = MathHelper::floor_double(entity->posX / 16.0);
	const int_t oldZ = MathHelper::floor_double(entity->posZ / 16.0);
	const int_t margin = PLATFORM_PLAYER_UPDATE_CHUNK_RANGE_BLOCKS;
	const int_t bx = MathHelper::floor_double(entity->posX);
	const int_t bz = MathHelper::floor_double(entity->posZ);
	const bool stalled = !entity->addedToChunk || !chunkExists(oldX, oldZ) ||
		!checkChunksExist(bx - margin, 0, bz - margin, bx + margin, WorldHeight::HEIGHT, bz + margin);
	if (!isActiveClientEntity(entity) && !entity->isDead && stalled)
	{
		detachEntityForWorldChange(entity);
		entity->setPositionAndRotation2(x, y, z, yaw, pitch, 0);
		entity->setPositionAndRotation(x, y, z, yaw, pitch);
		entity->lastTickPosX = entity->prevPosX = x;
		entity->lastTickPosY = entity->prevPosY = y;
		entity->lastTickPosZ = entity->prevPosZ = z;
		entitySpawnQueue.add(entity);
		MC_LOG_TRACE("net.entity", "resume-position id=%d oldChunk=%d,%d newChunk=%d,%d\n",
			entity->entityId, oldX, oldZ, MathHelper::floor_double(x / 16.0), MathHelper::floor_double(z / 16.0));
		return;
	}
#endif
	entity->setPositionAndRotation2(x, y, z, yaw, pitch, 3);
}

bool WorldClient::entityJoinedWorld(Entity *entity)
{
#if PLATFORM_PS2
	if (entity == nullptr || entity->isDead)
		return false;
	// World's player exception must only apply to the local camera. Remote
	// players must never attach to the shared EmptyChunk.
	if (!isActiveClientEntity(entity) && !chunkExists(
		MathHelper::floor_double(entity->posX / 16.0), MathHelper::floor_double(entity->posZ / 16.0)))
	{
		knownEntities.add(entity);
		entitySpawnQueue.add(entity);
		return false;
	}
#endif
	bool added = World::entityJoinedWorld(entity);
	knownEntities.add(entity);
	if (!added) entitySpawnQueue.add(entity);
	return added;
}

void WorldClient::setEntityDead(Entity *entity)
{
	if (entity == nullptr)
		return;

	const bool loaded = isLoadedEntityPointer(entity);
	// Remove by pointer before the entity ID can be reused by a later spawn.
	// A deferred entity is not in loadedEntityList, so it also needs an explicit
	// unload entry to preserve C++ ownership until updateEntities deletes it.
	entitySpawnQueue.remove(entity);
	knownEntities.remove(entity);
	World::setEntityDead(entity);
	if (!loaded)
		queueEntityForDestruction(entity);
#if PLATFORM_PS2
	MC_LOG_DEBUG("net.entity", "retired id=%d loaded=%d pending=%zu known=%zu\n",
		entity->entityId, loaded ? 1 : 0, entitySpawnQueue.size(), knownEntities.size());
#endif
}

void WorldClient::unloadEntities(const std::vector<Entity *> &list)
{
	// Java WorldClient keeps live network entities strongly referenced in
	// entityList/entitySpawnQueue when their chunk unloads. World::unloadEntities
	// cannot be used for those objects in C++ because World::updateEntities owns
	// and destroys entries queued there. Preserve them explicitly and detach them
	// from the active-tick list until their chunk becomes available again.
	std::vector<Entity *> destroyOnUnload;
	for (Entity *entity : list)
	{
		if (entity == nullptr)
			continue;

		auto loadedIt = std::find(loadedEntityList.begin(), loadedEntityList.end(), entity);
		if (loadedIt == loadedEntityList.end())
			continue;

		// The local player remains an active client object even if the server
		// unloads the column under it. Keep ticking it, but detach its chunk link.
		if (isActiveClientEntity(entity))
		{
			entity->addedToChunk = false;
			if (knownEntities.contains(entity))
				entitySpawnQueue.add(entity);
			continue;
		}

		if (!entity->isDead && knownEntities.contains(entity))
		{
			loadedEntityList.erase(loadedIt);
			untrackLoadedEntityPointer(entity);
			entityCountsDirty = true;
			entity->addedToChunk = false;
#if PLATFORM_PS2
			if (entity->isPlayer())
				playerEntities.erase(std::remove(playerEntities.begin(), playerEntities.end(), static_cast<EntityPlayer *>(entity)), playerEntities.end());
			MC_LOG_DEBUG("net.entity", "chunk-detach id=%d chunk=%d,%d\n", entity->entityId, entity->chunkCoordX, entity->chunkCoordZ);
#endif
			releaseEntitySkin(entity);
			continue;
		}

		destroyOnUnload.push_back(entity);
	}

	if (!destroyOnUnload.empty())
		World::unloadEntities(destroyOnUnload);
}

void WorldClient::obtainEntitySkin(Entity *entity)
{
	World::obtainEntitySkin(entity);
	entitySpawnQueue.remove(entity);
}

void WorldClient::releaseEntitySkin(Entity *entity)
{
	World::releaseEntitySkin(entity);
	if (!knownEntities.contains(entity))
		return;

	// Java only requeues live entities. Dead ones are removed from entityList
	// instead of being resurrected on the next client tick.
	if (!entity->isDead)
		entitySpawnQueue.add(entity);
	else
	{
		entitySpawnQueue.remove(entity);
		knownEntities.remove(entity);
	}
}

void WorldClient::onEntityRemoved(Entity *entity)
{
	World::onEntityRemoved(entity);
	// World is about to `delete entity`. Drop every non-owning reference so the next
	// tick()'s entitySpawnQueue flush can't dereference freed memory. Must run after
	// releaseEntitySkin (which re-inserts dying entities into entitySpawnQueue) and
	// right before the delete. Java never needed this: entities were GC-managed, so a
	// stale pointer in the queue was still a live object.
	for (Entity *candidate : knownEntities.valuesInIterationOrder())
	{
		if (candidate == nullptr || candidate == entity ||
		    candidate->getEntityClassID() != EntityArrow::CLASS_ID)
			continue;
		EntityArrow *arrow = static_cast<EntityArrow *>(candidate);
		if (arrow->owner == entity)
			arrow->owner = nullptr;
	}

	entitySpawnQueue.remove(entity);
	knownEntities.remove(entity);
	if (static_cast<Entity *>(entityHash->lookup(entity->entityId)) == entity)
		entityHash->removeObject(entity->entityId);
}

void WorldClient::detachEntityForWorldChange(Entity *entity)
{
	World::detachEntityForWorldChange(entity);
	if (entity == nullptr)
		return;

	// The base scrub clears every World-owned list, but this class keeps three
	// more non-owning sets, and ~WorldClient() RE-ADOPTS whatever is left in
	// knownEntities so the base destructor deletes it. An entity detached for a
	// world change (the local player on an MP dimension-change respawn) already
	// belongs to the world it crossed into, which destroys it through the normal
	// updateEntities graveyard; leaving it in the abandoned world's sets made
	// BOTH worlds free it -- the respawn "Undefined Instruction" crash, whose
	// dump pointed at the graveyard deleting an already-poisoned vtable
	// (PC=0 / slot 0x519, 2026-09-28).
	entitySpawnQueue.remove(entity);
	knownEntities.remove(entity);
	if (static_cast<Entity *>(entityHash->lookup(entity->entityId)) == entity)
		entityHash->removeObject(entity->entityId);
}

void WorldClient::addEntityToWorld(int_t entityId, Entity *entity)
{
	if (Entity *oldEntity = getEntityByID(entityId)) setEntityDead(oldEntity);

	// Assign the server ID before publishing the object; ownership sets use
	// pointer identity, while entityHash remains the authoritative ID lookup.
	entity->entityId = entityId;
	knownEntities.add(entity);
	const bool joined = entityJoinedWorld(entity);
	if (!joined)
	{
		entitySpawnQueue.add(entity);
#if PLATFORM_PS2
		MC_LOG_DEBUG("net.entity", "spawn queued id=%d chunk=%d,%d pending=%zu known=%zu\n",
			entityId, MathHelper::floor_double(entity->posX / 16.0),
			MathHelper::floor_double(entity->posZ / 16.0), entitySpawnQueue.size(), knownEntities.size());
#endif
	}
	else
	{
#if PLATFORM_PS2
		MC_LOG_DEBUG("net.entity", "spawn joined id=%d loaded=%zu known=%zu\n",
			entityId, loadedEntityList.size(), knownEntities.size());
#endif
	}
	entityHash->addKey(entityId, entity);
}

Entity *WorldClient::getEntityByID(int_t entityId)
{
	Entity *entity = entityHash != nullptr ? static_cast<Entity *>(entityHash->lookup(entityId)) : nullptr;
	if (entity != nullptr)
		return entity;
	return World::getEntityByID(entityId);
}

Entity *WorldClient::removeEntityFromWorld(int_t entityId)
{
	Entity *entity = static_cast<Entity *>(entityHash->removeObject(entityId));
	if (entity != nullptr)
	{
		const bool wasLoaded = isLoadedEntityPointer(entity);
		const bool wasPending = entitySpawnQueue.contains(entity);
#if PLATFORM_PS2
		MC_LOG_DEBUG("net.entity", "destroy received id=%d loaded=%d pending=%d known=%zu\n",
			entityId, wasLoaded ? 1 : 0, wasPending ? 1 : 0, knownEntities.size());
#endif
		knownEntities.remove(entity);
		setEntityDead(entity);
	}
	else
		MC_LOG_DEBUG("net.entity", "destroy unknown id=%d\n", entityId);
	return entity;
}

bool WorldClient::setBlockMetadata(int_t x, int_t y, int_t z, int_t metadata)
{
	int_t oldId = getBlockId(x, y, z), oldMetadata = getBlockMetadata(x, y, z);
	if (!World::setBlockMetadata(x, y, z, metadata)) return false;
	pendingBlockChanges.push_back(new WorldBlockPositionType(this, x, y, z, oldId, oldMetadata));
	return true;
}

bool WorldClient::setBlockAndMetadata(int_t x, int_t y, int_t z, int_t blockId, int_t metadata)
{
	int_t oldId = getBlockId(x, y, z), oldMetadata = getBlockMetadata(x, y, z);
	if (!World::setBlockAndMetadata(x, y, z, blockId, metadata)) return false;
	pendingBlockChanges.push_back(new WorldBlockPositionType(this, x, y, z, oldId, oldMetadata));
	return true;
}

bool WorldClient::setBlock(int_t x, int_t y, int_t z, int_t blockId)
{
	int_t oldId = getBlockId(x, y, z), oldMetadata = getBlockMetadata(x, y, z);
	if (!World::setBlock(x, y, z, blockId)) return false;
	pendingBlockChanges.push_back(new WorldBlockPositionType(this, x, y, z, oldId, oldMetadata));
	return true;
}

bool WorldClient::setBlockAndMetadataAndInvalidate(int_t x, int_t y, int_t z, int_t blockId, int_t metadata)
{
	invalidateBlockReceiveRegion(x, y, z, x, y, z);
	if (!World::setBlockAndMetadata(x, y, z, blockId, metadata)) return false;
	notifyBlockChange(x, y, z, blockId);
	return true;
}

void WorldClient::sendQuittingDisconnectingPacket()
{
	sendQueue->sendPacketAndFlush(new Packet255KickDisconnect("Quitting"));
}

void WorldClient::updateWeather()
{
	if (worldProvider->hasNoSky) return;
	if (field_27172_i > 0)
		--field_27172_i;
	prevRainingStrength = rainingStrength;
	rainingStrength = worldInfo->getRaining()
		? (float)((double)rainingStrength + 0.01)
		: (float)((double)rainingStrength - 0.01);
	if (rainingStrength < 0.0f) rainingStrength = 0.0f;
    if (rainingStrength > 1.0f) rainingStrength = 1.0f;
	prevThunderingStrength = thunderingStrength;
	thunderingStrength = worldInfo->getThundering()
		? (float)((double)thunderingStrength + 0.01)
		: (float)((double)thunderingStrength - 0.01);
	if (thunderingStrength < 0.0f) thunderingStrength = 0.0f;
    if (thunderingStrength > 1.0f) thunderingStrength = 1.0f;
}
