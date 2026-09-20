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
	// before retrying its entities.
#if PLATFORM_PS2 && PLATFORM_MP_DEFERRED_CHUNKS
	sendQueue->processReadPackets();
	std::vector<Entity *> spawnCandidates = entitySpawnQueue.valuesInIterationOrder();
	selectClientEntityRetryBatch(spawnCandidates, entityRetryCursor, 24);
	promoteDeferredChunks(&spawnCandidates);
	trimClientChunkCache();
#else
	std::vector<Entity *> spawnCandidates = entitySpawnQueue.valuesInIterationOrder();
	if (spawnCandidates.size() > 10)
		spawnCandidates.resize(10);
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


#if !(PLATFORM_PS2 && PLATFORM_MP_DEFERRED_CHUNKS)
	sendQueue->processReadPackets();
#if PLATFORM_MP_DEFERRED_CHUNKS
	promoteDeferredChunks();
	trimClientChunkCache();
#endif
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
#if PLATFORM_MP_DEFERRED_CHUNKS
		// A stock Beta server sends a 21x21 (441 chunk) window. The Wii renderer
		// only needs its bounded cache, so never materialize the distant columns.
		if (!shouldKeepChunk(chunkX, chunkZ))
			return;
#endif
		clientChunkProvider->prepareChunk(chunkX, chunkZ);
	}
	else
	{
		forgetDeferredChunk(chunkX, chunkZ);
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
    enforceDeferredChunkBudget();
#else
    (void)chunkX; (void)chunkZ; (void)includeInitialize;
    (void)primaryMask; (void)addMask; (void)compressed;
#endif
}

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
    enforceDeferredChunkBudget();
#else
    (void)chunkX; (void)chunkZ;
#endif
}

void WorldClient::enforceDeferredChunkBudget()
{
#if PLATFORM_MP_DEFERRED_CHUNKS
    constexpr std::size_t MAX_BYTES = PLATFORM_MP_COMPRESSED_CHUNK_CACHE_BYTES;
    constexpr std::size_t MAX_CHUNKS = PLATFORM_MP_MAX_DEFERRED_CHUNKS;
    auto overLimit = [&]()
    {
        return deferredChunkBytes > MAX_BYTES || deferredChunks.size() > MAX_CHUNKS;
    };

    const bool startedOverLimit = overLimit();
    if (startedOverLimit && !deferredChunkBudgetExceeded)
        ++deferredChunkBudgetOverflows;
    deferredChunkBudgetExceeded = startedOverLimit;

    int_t centerX = 0;
    int_t centerZ = 0;
    const bool havePlayer = !playerEntities.empty() && playerEntities[0] != nullptr;
    if (havePlayer)
    {
        centerX = JavaArithmetic::intShr(MathHelper::floor_double(playerEntities[0]->posX), 4);
        centerZ = JavaArithmetic::intShr(MathHelper::floor_double(playerEntities[0]->posZ), 4);
    }

    while (overLimit() && !deferredChunks.empty())
    {
        auto victim = deferredChunks.end();
        bool victimHasBase = true;
        bool victimInWorkingSet = true;
        bool victimResident = false;
        long_t victimDistance = -1;
        ulong_t victimStamp = std::numeric_limits<ulong_t>::max();

        for (auto it = deferredChunks.begin(); it != deferredChunks.end(); ++it)
        {
            const DeferredChunk &candidate = it->second;
            const bool hasBase = !candidate.compressed.empty();
            const long_t distance = havePlayer
                ? getChunkDistance(candidate.chunkX, candidate.chunkZ, centerX, centerZ)
                : 0;
            const bool inWorkingSet = havePlayer &&
                distance <= static_cast<long_t>(PLATFORM_CHUNK_UNLOAD_RADIUS);
            const bool resident = clientChunkProvider != nullptr &&
                clientChunkProvider->hasChunk(candidate.chunkX, candidate.chunkZ);

            // First discard entries that cannot reconstruct a column, then
            // the farthest data outside the PS2 working set. Under extreme
            // pressure, an already-resident column is a better victim than a
            // nearby column still waiting to be promoted.
            const bool better = victim == deferredChunks.end() ||
                (hasBase != victimHasBase && !hasBase) ||
                (hasBase == victimHasBase && inWorkingSet != victimInWorkingSet && !inWorkingSet) ||
                (hasBase == victimHasBase && inWorkingSet == victimInWorkingSet &&
                 distance != victimDistance && distance > victimDistance) ||
                (hasBase == victimHasBase && inWorkingSet == victimInWorkingSet &&
                 distance == victimDistance && resident != victimResident && resident) ||
                (hasBase == victimHasBase && inWorkingSet == victimInWorkingSet &&
                 distance == victimDistance && resident == victimResident &&
                 candidate.stamp < victimStamp);
            if (!better)
                continue;

            victim = it;
            victimHasBase = hasBase;
            victimInWorkingSet = inWorkingSet;
            victimResident = resident;
            victimDistance = distance;
            victimStamp = candidate.stamp;
        }

        if (victim == deferredChunks.end())
            break;
        deferredChunkBytes -= victim->second.compressed.size();
        for (const DeferredMapUpdate &update : victim->second.sectionUpdates)
            deferredChunkBytes -= update.compressed.size();
        deferredBlockChangeCount -= victim->second.changes.size();
        deferredChunks.erase(victim);
        ++deferredChunkEvictions;
    }

    deferredChunkBudgetExceeded = overLimit();
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
    markBlocksDirty(minX, 0, minZ, maxX, WorldHeight::HEIGHT, maxZ);
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
}

bool WorldClient::shouldKeepChunk(int_t chunkX, int_t chunkZ) const
{
#if PLATFORM_MP_DEFERRED_CHUNKS
	if (playerEntities.empty() || playerEntities[0] == nullptr)
		return false;
	const EntityPlayer *player = playerEntities[0];
	const int_t centerX = JavaArithmetic::intShr(JavaArithmetic::doubleToInt(std::floor(player->posX)), 4);
	const int_t centerZ = JavaArithmetic::intShr(JavaArithmetic::doubleToInt(std::floor(player->posZ)), 4);
	return getChunkDistance(chunkX, chunkZ, centerX, centerZ) <= PLATFORM_CHUNK_UNLOAD_RADIUS;
#else
	(void)chunkX;
	(void)chunkZ;
	return true;
#endif
}

void WorldClient::trimClientChunkCache()
{
#if PLATFORM_MP_DEFERRED_CHUNKS
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
	return static_cast<Entity *>(entityHash->lookup(entityId));
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
