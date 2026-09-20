#include "World.h"
#include "platform/Log.h"
#include "platform/WorldLoadTrace.h"
#include "platform/Diagnostics.h"
#include "platform/PlatformTuning.h"
#if PLATFORM_PC_LEGACY
#include "pc/world/PcLegacyTickScheduler.h"
#endif
#if PLATFORM_PROFILE_STREAMING && MC_LOG_LEVEL >= 2
// Defined in client/Minecraft.cpp; see the matching comment in ChunkProvider.cpp
// for the sibling counters this timer family belongs to. TickUpdates() is the last
// unmeasured piece of World::tick() -- populate/lighting/unloadSave together cannot
// account for the observed 1.7-1.9s "world" spikes, and this is the one call in
// tick() with no time bound at all: it drains up to 1000 scheduled updates per
// call (see World::TickUpdates below), each of which can run an arbitrary
// Block::updateTick() and schedule more.
// scheduledTickTreeSet.size() sampled right before TickUpdates() drains it --
// confirms whether a tickUpdates spike is the 1000-per-call cap saturating.
// Around SpawnerAnimals::performSpawning() below -- the next untimed candidate
// once populate/lighting/unloadSave/tickUpdates all measured near-zero during
// the observed "world" spikes.
// mobSpawn also measured near-zero, which exonerates every named sub-phase of
// World::tick() -- yet the "world" bucket itself still showed 1.7-1.9s spikes
// (2026-08-12 log). The one call left inside tick() with no timer at all is
// saveWorld() -> saveLevel(), gated behind autosavePeriod rather than firing
// every tick, which is exactly why it evaded the earlier per-tick suspects.
// Split into its two halves (World.cpp's saveLevel(), below) since it's not
// yet known which one is actually expensive on console storage.
#endif

#include <limits>
#include <algorithm>
#include <cmath>
#include <unordered_set>
#include <vector>
#if PLATFORM_RANDOM_TICK_PROFILE_INTERVAL > 0
#include <cstdio> // the randomBlocks per-branch breakdown
#endif

#include "java/System.h"
#include "java/Arithmetic.h"
#include "IBlockAccess.h"
#include "WorldProvider.h"
#include "WorldType.h"
#include "WorldHeight.h"
#include "WorldInfo.h"
#include "WorldSettings.h"
#include "MapStorage.h"
#include "ISaveHandler.h"
#include "ChunkProvider.h"
#include "EntityPlayer.h"
#include "ChunkProviderLoadOrGenerate.h"
#include "MathHelper.h"
#include "IChunkProvider.h"
#include "SpawnListEntry.h"
#include "IProgressUpdate.h"
#include "Chunk.h"
#include "ChunkPosition.h"
#include "ExtendedBlockStorage.h"
#include "Material.h"
#include "Block.h"
#include "BlockGrass.h"
#include "BlockTallGrass.h"
#include "BlockVine.h"
#include "IWorldAccess.h"
#include "EnumSkyBlock.h"
#include "Vec3D.h"
#include "VillageCollection.h"
#include "VillageSiege.h"
#include "Entity.h"
#include "AxisAlignedBB.h"
#include "WorldChunkManager.h"
#include "BiomeGenBase.h"
#include "NextTickListEntry.h"
#include "TileEntity.h"
#include "ThreadedFileIOBase.h"
#include "BlockFire.h"
#include "BlockFluid.h"
#include "Explosion.h"
#include "MetadataChunkBlock.h"
#include "SpawnerAnimals.h"
#include "EntityMob.h"
#include "EntityCreature.h"
#include "EntityAnimal.h"
#include "EntityWaterMob.h"
#include "ChunkCoordIntPair.h"
#include "EntityLightningBolt.h"
#include "ChunkCache.h"
#include "PathFinder.h"
#include "ChunkCoordinates.h"
#include "MovingObjectPosition.h"
#include "PathEntity.h"
#include "MapDataBase.h"
#include "NBTTagCompound.h"
#include "EnumCreatureType.h"
#include "Container.h"
#include "IInventory.h"
#include "ItemStack.h"
#include "EnumStatus.h"
#include "IChunkLoader.h"
#include "BlockLeaves.h"
#include "client/Minecraft.h"
#include "platform/PlatformCompat.h"
#include "platform/Profiler.h"
#include "platform/WorkProfiler.h"
#include "platform/ExtendedProfiler.h"
#if PLATFORM_FAST_BLOCK_COLLISIONS || PLATFORM_EARLY_COLLISION_EXIT || PLATFORM_FLOAT_COLLISION_SWEEP
#include "platform/world/PlatformBlockCollisionSweeper.h"
#include "platform/world/StreamingFrameBudget.h"
#endif

#if PLATFORM_BOUNDED_WORLD
static int_t platformFindTopSpawnBlockY(World *world, int_t x, int_t z)
{
    // Find the highest solid block at the chosen spawn column. Vanilla Beta
    // keeps SpawnY around 64 and lets the player fall/resolve collision, but a
    // bounded chunk cache can delay player ticking. A resolved surface Y
    // prevents the player from appearing far above the terrain while the world
    // finishes loading.
    for (int_t y = WorldHeight::MAX_Y; y >= 0; --y)
    {
        int_t id = world->getBlockId(x, y, z);
        if (id <= 0)
            continue;

        Block *block = Block::blocksList[id];
        if (block != nullptr && block->blockMaterial != nullptr && block->blockMaterial->getIsSolid())
            return y;
    }

    return 64;
}
#endif

// Static initialization
int World::lightingUpdatesScheduled = 0;
static constexpr int_t UPDATE_LCG_INCREMENT = 1013904223;

static std::uint64_t packChunkCoordKey(int_t x, int_t z)
{
    return (static_cast<std::uint64_t>(static_cast<std::uint32_t>(x)) << 32)
         | static_cast<std::uint32_t>(z);
}

static int_t unpackChunkCoordX(std::uint64_t key)
{
    return JavaArithmetic::intFromBits(static_cast<uint_t>(key >> 32));
}

static int_t unpackChunkCoordZ(std::uint64_t key)
{
    return JavaArithmetic::intFromBits(static_cast<uint_t>(key));
}

// Java's World only drops references to removed entities and lets the GC decide when
// to destroy them.  Minecraft keeps independent strong references to the local player
// and camera, so the C++ World must not delete either object while those references are
// still active.
static bool isActiveClientEntity(Entity *entity)
{
    Minecraft *mc = Minecraft::getMinecraft();
    return mc != nullptr &&
           (static_cast<void *>(mc->thePlayer) == static_cast<void *>(entity) ||
            static_cast<void *>(mc->renderViewEntity) == static_cast<void *>(entity));
}

static void deleteWorldOwnedEntity(Entity *entity)
{
    if (!isActiveClientEntity(entity))
        delete entity;
}

static void pushUniqueTileEntity(std::vector<TileEntity *> &list, TileEntity *tileEntity)
{
    if (tileEntity != nullptr && std::find(list.begin(), list.end(), tileEntity) == list.end())
        list.push_back(tileEntity);
}

void World::notifyTileEntityRenderersRemoved(TileEntity *tileEntity)
{
    if (tileEntity == nullptr)
        return;
    for (IWorldAccess *access : worldAccesses)
        access->doNothingWithTileEntity(tileEntity->xCoord, tileEntity->yCoord, tileEntity->zCoord, tileEntity);
}

void World::detachTileEntityForChunkUnload(TileEntity *tileEntity)
{
    if (tileEntity == nullptr)
        return;
    notifyTileEntityRenderersRemoved(tileEntity);
    loadedTileEntityList.erase(std::remove(loadedTileEntityList.begin(), loadedTileEntityList.end(), tileEntity), loadedTileEntityList.end());
    tileEntitiesToAdd.erase(std::remove(tileEntitiesToAdd.begin(), tileEntitiesToAdd.end(), tileEntity), tileEntitiesToAdd.end());
}

// Java kept a removed TileEntity alive for as long as an open GuiFurnace/
// GuiChest referenced it: the container went on writing into an object the
// world no longer owned, and the player was eventually kicked out by the
// isUsableByPlayer() check in EntityPlayer::onUpdate. Here the same removal
// frees the object -- along with the ItemStacks it owns -- so every path that
// deletes a TileEntity has to close the screens pointing at it FIRST, while the
// pointer is still dereferenceable. Otherwise the next isUsableByPlayer() runs
// through a freed vtable (the Wii ISI / PS2 unaligned-PC crashes) and, when it
// happens to survive, Chunk::getChunkBlockTileEntity quietly manufactures an
// empty replacement -- which is what looked like "the furnace ate my items".
void World::closeContainersUsing(TileEntity *tileEntity)
{
    IInventory *iinventory = dynamic_cast<IInventory *>(tileEntity);
    if (iinventory == nullptr)
        return;
    for (EntityPlayer *player : playerEntities)
    {
        if (player == nullptr || player->craftingInventory == nullptr)
            continue;
        if (player->craftingInventory == player->inventorySlots)
            continue;
        if (player->craftingInventory->usesInventory(iinventory))
            player->closeScreen();
    }
}


WorldChunkManager* World::getWorldChunkManager()
{
    return worldProvider->worldChunkMgr;
}

World::~World()
{
    // The same object can appear in the world list, a chunk list and an unload
    // queue.  Destroy each allocation once, after providers no longer use it.
    std::unordered_set<Entity *> entities;
    entities.insert(loadedEntityList.begin(), loadedEntityList.end());
    entities.insert(unloadedEntityList.begin(), unloadedEntityList.end());
    entities.insert(weatherEffects.begin(), weatherEffects.end());
    entities.insert(playerEntities.begin(), playerEntities.end());

    std::unordered_set<TileEntity *> tileEntities;
    tileEntities.insert(loadedTileEntityList.begin(), loadedTileEntityList.end());
    tileEntities.insert(tileEntitiesToAdd.begin(), tileEntitiesToAdd.end());

    delete pathfinderCache;
    pathfinderCache = nullptr;

    delete chunkProvider;
    chunkProvider = nullptr;

    for (Entity *entity : entities)
        deleteWorldOwnedEntity(entity);
    for (TileEntity *tileEntity : tileEntities)
        delete tileEntity;
    // lightingToUpdate now holds MetadataChunkBlock by value; nothing to free.
    for (NextTickListEntry *entry : scheduledTickTreeSet)
        delete entry;
#if PLATFORM_PC_LEGACY
    delete pcLegacyTickScheduler;
    pcLegacyTickScheduler = nullptr;
#endif

    delete villageSiegeObj;
    villageSiegeObj = nullptr;
    delete villageCollectionObj;
    villageCollectionObj = nullptr;
    delete worldProvider;
    delete worldInfo;
    if (ownsMapStorage)
        delete mapStorage;
    if (ownsSaveHandler)
        delete saveHandler;
}

World::World(ISaveHandler* saveHandler, const jstring& name, WorldProvider* worldProvider, long_t seed)
{
    scheduledUpdatesAreImmediate = false;
    lightingToUpdate.clear();
    loadedEntityList.clear();
    entityCountsDirty = true;
    unloadedEntityList.clear();
    scheduledTickTreeSet.clear();
    scheduledTickSet.clear();
    scheduledTickOrder.clear();
    loadedTileEntityList.clear();
    tileEntitiesToAdd.clear();
    playerEntities.clear();
    weatherEffects.clear();
    field_1019_F = 0xffffffL;
    skylightSubtracted = 0;
    thunderTime = 0;
    field_27172_i = 0;
    editingBlocks = false;
    lockTimestamp = System::currentTimeMillis();
    autosavePeriod = PLATFORM_AUTOSAVE_PERIOD_TICKS;
    isNewWorld = false;
    worldAccesses.clear();
    collidingBoundingBoxes.clear();
    lightingUpdatesCounter = 0;
    spawnHostileMobs = true;
    spawnPeacefulMobs = true;
    positionsToUpdate.clear();
    soundCounter = rand.nextInt(12000);
    entitiesWithinAABBExcludingEntity.clear();
    multiplayerWorld = false;
    
    this->saveHandler = saveHandler;
    this->worldInfo = new WorldInfo(seed, name);
    this->worldProvider = worldProvider;
    this->mapStorage = new MapStorage(saveHandler);
    this->villageCollectionObj = new VillageCollection(this);
    this->villageSiegeObj = new VillageSiege(this);
    
    worldProvider->registerWorld(this);
    chunkProvider = getChunkProvider();
    calculateInitialSkylight();
    initializeWeatherStrengths();
}

World::World(World* world, WorldProvider* worldProvider)
{
    scheduledUpdatesAreImmediate = false;
    lightingToUpdate.clear();
    loadedEntityList.clear();
    entityCountsDirty = true;
    unloadedEntityList.clear();
    scheduledTickTreeSet.clear();
    scheduledTickSet.clear();
    scheduledTickOrder.clear();
    loadedTileEntityList.clear();
    tileEntitiesToAdd.clear();
    playerEntities.clear();
    weatherEffects.clear();
    field_1019_F = 0xffffffL;
    skylightSubtracted = 0;
    thunderTime = 0;
    field_27172_i = 0;
    editingBlocks = false;
    lockTimestamp = System::currentTimeMillis();
    autosavePeriod = PLATFORM_AUTOSAVE_PERIOD_TICKS;
    isNewWorld = false;
    worldAccesses.clear();
    collidingBoundingBoxes.clear();
    lightingUpdatesCounter = 0;
    spawnHostileMobs = true;
    spawnPeacefulMobs = true;
    naturalMobSpawningEnabled = world->naturalMobSpawningEnabled;
    positionsToUpdate.clear();
    soundCounter = rand.nextInt(12000);
    entitiesWithinAABBExcludingEntity.clear();
    multiplayerWorld = false;
    
    this->lockTimestamp = world->lockTimestamp;
    this->saveHandler = world->saveHandler;
    this->ownsSaveHandler = world->ownsSaveHandler;
    world->ownsSaveHandler = false;
    this->worldInfo = new WorldInfo(world->worldInfo);
    this->mapStorage = new MapStorage(saveHandler);
    this->worldProvider = worldProvider;
    this->villageCollectionObj = new VillageCollection(this);
    this->villageSiegeObj = new VillageSiege(this);
    
    worldProvider->registerWorld(this);
    chunkProvider = getChunkProvider();
    calculateInitialSkylight();
    initializeWeatherStrengths();
}

void World::setMapStorage(MapStorage *storage)
{
    if (ownsMapStorage)
        delete mapStorage;
    mapStorage = storage;
    ownsMapStorage = false;
}

World::World(ISaveHandler* saveHandler, const jstring& name, WorldSettings* settings)
    : World(saveHandler, name, settings, nullptr)
{
}

World::World(ISaveHandler* saveHandler, const jstring& name, WorldSettings* settings, WorldProvider* worldProvider)
{
    WORLD_LOAD_STAGE("World ctor");
    WorldLoadTrace::step("fields");
    scheduledUpdatesAreImmediate = false;
    lightingToUpdate.clear();
    loadedEntityList.clear();
    entityCountsDirty = true;
    unloadedEntityList.clear();
    scheduledTickTreeSet.clear();
    scheduledTickSet.clear();
    scheduledTickOrder.clear();
    loadedTileEntityList.clear();
    tileEntitiesToAdd.clear();
    playerEntities.clear();
    weatherEffects.clear();
    field_1019_F = 0xffffffL;
    skylightSubtracted = 0;
    thunderTime = 0;
    field_27172_i = 0;
    editingBlocks = false;
    lockTimestamp = System::currentTimeMillis();
    autosavePeriod = PLATFORM_AUTOSAVE_PERIOD_TICKS;
    isNewWorld = false;
    worldAccesses.clear();
    collidingBoundingBoxes.clear();
    lightingUpdatesCounter = 0;
    spawnHostileMobs = true;
    spawnPeacefulMobs = true;
    positionsToUpdate.clear();
    soundCounter = rand.nextInt(12000);
    entitiesWithinAABBExcludingEntity.clear();
    multiplayerWorld = false;

    this->saveHandler = saveHandler;
    WorldLoadTrace::step("mapStorage");
    this->mapStorage = new MapStorage(saveHandler);
    WorldLoadTrace::step("loadWorldInfo");
    this->worldInfo = saveHandler->loadWorldInfo();
    this->isNewWorld = (this->worldInfo == nullptr);

    WorldLoadTrace::step("worldProvider");
    if (worldProvider != nullptr)
    {
        this->worldProvider = worldProvider;
    }
    else if (this->worldInfo != nullptr && this->worldInfo->getDimension() != 0)
    {
        this->worldProvider = WorldProvider::getProviderForDimension(this->worldInfo->getDimension());
    }
    else
    {
        this->worldProvider = WorldProvider::getProviderForDimension(0);
    }

    WorldLoadTrace::step("worldInfo");
    bool newWorld = false;
    if (this->worldInfo == nullptr)
    {
        this->worldInfo = new WorldInfo(settings, name);
        newWorld = true;
    }
    else
    {
        this->worldInfo->setWorldName(name);
    }

    WorldLoadTrace::step("villages");
    this->villageCollectionObj = new VillageCollection(this);
    this->villageSiegeObj = new VillageSiege(this);
    WorldLoadTrace::step("registerWorld");
    this->worldProvider->registerWorld(this);
    WorldLoadTrace::step("chunkProvider");
    this->chunkProvider = getChunkProvider();

    WorldLoadTrace::step("initialSpawn");
    if (newWorld)
        getInitialSpawnLocation();

    WorldLoadTrace::step("initialSkylight");
    calculateInitialSkylight();
    WorldLoadTrace::step("weather");
    initializeWeatherStrengths();
}

World::World(ISaveHandler* saveHandler, const jstring& name, long_t seed)
    : World(saveHandler, name, seed, nullptr)
{
}

World::World(ISaveHandler* saveHandler, const jstring& name, long_t seed, WorldProvider* worldProvider)
{
    WORLD_LOAD_STAGE("World ctor");
    WorldLoadTrace::step("fields");
    scheduledUpdatesAreImmediate = false;
    lightingToUpdate.clear();
    loadedEntityList.clear();
    entityCountsDirty = true;
    unloadedEntityList.clear();
    scheduledTickTreeSet.clear();
    scheduledTickSet.clear();
    scheduledTickOrder.clear();
    loadedTileEntityList.clear();
    tileEntitiesToAdd.clear();
    playerEntities.clear();
    weatherEffects.clear();
    field_1019_F = 0xffffffL;
    skylightSubtracted = 0;
    thunderTime = 0;
    field_27172_i = 0;
    editingBlocks = false;
    lockTimestamp = System::currentTimeMillis();
    autosavePeriod = PLATFORM_AUTOSAVE_PERIOD_TICKS;
    isNewWorld = false;
    worldAccesses.clear();
    collidingBoundingBoxes.clear();
    lightingUpdatesCounter = 0;
    spawnHostileMobs = true;
    spawnPeacefulMobs = true;
    positionsToUpdate.clear();
    soundCounter = rand.nextInt(12000);
    entitiesWithinAABBExcludingEntity.clear();
    multiplayerWorld = false;
    
    this->saveHandler = saveHandler;
    WorldLoadTrace::step("mapStorage");
    this->mapStorage = new MapStorage(saveHandler);
    WorldLoadTrace::step("loadWorldInfo");
    this->worldInfo = saveHandler->loadWorldInfo();
    this->isNewWorld = (this->worldInfo == nullptr);

    WorldLoadTrace::step("worldProvider");
    if (worldProvider != nullptr)
    {
        this->worldProvider = worldProvider;
    }
    else if (this->worldInfo != nullptr && this->worldInfo->getDimension() != 0)
    {
        this->worldProvider = WorldProvider::getProviderForDimension(this->worldInfo->getDimension());
    }
    else
    {
        this->worldProvider = WorldProvider::getProviderForDimension(0);
    }
    
    WorldLoadTrace::step("worldInfo");
    bool flag = false;
    if (this->worldInfo == nullptr)
    {
        this->worldInfo = new WorldInfo(seed, name);
        flag = true;
    }
    else
    {
        this->worldInfo->setWorldName(name);
    }
    
    WorldLoadTrace::step("villages");
    this->villageCollectionObj = new VillageCollection(this);
    this->villageSiegeObj = new VillageSiege(this);
    WorldLoadTrace::step("registerWorld");
    this->worldProvider->registerWorld(this);
    WorldLoadTrace::step("chunkProvider");
    this->chunkProvider = getChunkProvider();
    
    WorldLoadTrace::step("initialSpawn");
    if (flag)
    {
        getInitialSpawnLocation();
    }
    
    WorldLoadTrace::step("initialSkylight");
    calculateInitialSkylight();
    WorldLoadTrace::step("weather");
    initializeWeatherStrengths();
}

SpawnListEntry *World::getRandomMob(const EnumCreatureType &type, int_t x, int_t y, int_t z)
{
    if (chunkProvider == nullptr)
        return nullptr;

    std::vector<SpawnListEntry> *spawnList = chunkProvider->getPossibleCreatures(type, x, y, z);
    if (spawnList == nullptr || spawnList->empty())
        return nullptr;

    int_t totalWeight = 0;
    for (const SpawnListEntry &entry : *spawnList)
        totalWeight += entry.spawnRarityRate;
    if (totalWeight <= 0)
        return nullptr;

    int_t selection = rand.nextInt(totalWeight);
    for (SpawnListEntry &entry : *spawnList)
    {
        selection -= entry.spawnRarityRate;
        if (selection < 0)
            return &entry;
    }
    return nullptr;
}

ChunkPosition *World::findClosestStructure(const jstring &name, int_t x, int_t y, int_t z)
{
    if (chunkProvider == nullptr)
        return nullptr;
    return chunkProvider->findClosestStructure(this, name, x, y, z);
}

IChunkProvider* World::createChunkProvider()
{
    return getChunkProvider();
}

IChunkProvider* World::getChunkProvider()
{
    IChunkLoader* chunkLoader = saveHandler->getChunkLoader(worldProvider);
    return new ChunkProvider(this, chunkLoader, worldProvider->getChunkProvider());
}

void World::generateSpawnPoint()
{
    if (!worldProvider->canRespawnHere())
    {
        worldInfo->setSpawn(0, worldProvider->getAverageGroundLevel(), 0);
        return;
    }

    findingSpawnPoint = true;
    WORLD_LOAD_STAGE("generateSpawnPoint");

#if PLATFORM_BOUNDED_WORLD
    // Bounded console generators cannot afford vanilla's up-to-1000 synchronous
    // chunk probes.  Keep the 1.2.5 biome-guided initial position, but cap the
    // coordinate probes and resolve a safe local surface Y.
#endif
    WorldChunkManager *manager = worldProvider->worldChunkMgr;
    std::vector<BiomeGenBase *> &spawnBiomes = manager->getBiomesToSpawnIn();
    Random spawnRandom(getSeed());
    WorldLoadTrace::step("findBiomePosition");
#if defined(PS2_PLATFORM)
    ChunkPosition *position = manager->findBiomePosition(0, 0, 64, spawnBiomes, spawnRandom);
#else
    ChunkPosition *position = manager->findBiomePosition(0, 0, 256, spawnBiomes, spawnRandom);
#endif

    int_t spawnX = 0;
    int_t spawnY = worldProvider->getAverageGroundLevel();
    int_t spawnZ = 0;
    if (position != nullptr)
    {
        spawnX = position->x;
        spawnZ = position->z;
        delete position;
    }

#if defined(PS2_PLATFORM)
    // Hardware PS2 cannot afford vanilla's synchronous spawn probing here.
    // Every canCoordinateBeSpawn() can force another complete chunk generation
    // before the loading screen is visible. Generate only the biome-guided
    // candidate chunk and find the best spawn column inside that same 16x16.
    WorldLoadTrace::step("spawnCandidateChunk");
    const int_t spawnChunkX = JavaArithmetic::intShr(spawnX, 4);
    const int_t spawnChunkZ = JavaArithmetic::intShr(spawnZ, 4);
    const int_t chunkWorldX = JavaArithmetic::intShl(spawnChunkX, 4);
    const int_t chunkWorldZ = JavaArithmetic::intShl(spawnChunkZ, 4);
    const int_t startLocalX = spawnX & 15;
    const int_t startLocalZ = spawnZ & 15;

    platformHardwareCheckpoint("before spawn candidate chunk");
    Chunk *spawnChunk = getChunkFromChunkCoords(spawnChunkX, spawnChunkZ);
    platformHardwareCheckpoint("after spawn candidate chunk");
    if (spawnChunk != nullptr)
    {
        bool foundGrass = false;
        int_t fallbackLocalX = startLocalX;
        int_t fallbackLocalZ = startLocalZ;
        int_t fallbackY = -1;

        WorldLoadTrace::step("scanSpawnChunk");
        for (int_t dz = 0; dz < 16 && !foundGrass; ++dz)
        {
            const int_t localZ = (startLocalZ + dz) & 15;
            for (int_t dx = 0; dx < 16; ++dx)
            {
                const int_t localX = (startLocalX + dx) & 15;
                const int_t surfaceY = spawnChunk->getHeightValue(localX, localZ) - 1;
                if (surfaceY < 0 || surfaceY >= WorldHeight::HEIGHT)
                    continue;

                const int_t blockId = spawnChunk->getBlockID(localX, surfaceY, localZ);
                if (blockId <= 0 || blockId >= Block::BLOCK_REGISTRY_SIZE)
                    continue;

                Block *block = Block::blocksList[blockId];
                if (fallbackY < 0 && block != nullptr && block->blockMaterial != nullptr &&
                    block->blockMaterial->getIsSolid())
                {
                    fallbackLocalX = localX;
                    fallbackLocalZ = localZ;
                    fallbackY = surfaceY;
                }

                if (blockId == Block::grass->blockID)
                {
                    spawnX = JavaArithmetic::intAdd(chunkWorldX, localX);
                    spawnZ = JavaArithmetic::intAdd(chunkWorldZ, localZ);
                    spawnY = surfaceY;
                    foundGrass = true;
                    break;
                }
            }
        }

        if (!foundGrass && fallbackY >= 0)
        {
            spawnX = JavaArithmetic::intAdd(chunkWorldX, fallbackLocalX);
            spawnZ = JavaArithmetic::intAdd(chunkWorldZ, fallbackLocalZ);
            spawnY = fallbackY;
        }
    }
#elif PLATFORM_BOUNDED_WORLD
    WorldLoadTrace::step("canCoordinateBeSpawn");
    for (int_t attempts = 0; attempts < 8 && !worldProvider->canCoordinateBeSpawn(spawnX, spawnZ); ++attempts)
    {
		MC_LOG_DEBUG("world", "spawn probe %d at %d,%d\n", (int)attempts, (int)spawnX, (int)spawnZ);
		spawnX = JavaArithmetic::intAdd(spawnX, spawnRandom.nextIntDifference(64));
		spawnZ = JavaArithmetic::intAdd(spawnZ, spawnRandom.nextIntDifference(64));
    }
    WorldLoadTrace::step("findTopSpawnBlockY");
    spawnY = platformFindTopSpawnBlockY(this, spawnX, spawnZ);
#else
    WorldLoadTrace::step("canCoordinateBeSpawn");
    for (int_t attempts = 0; attempts < 1000 && !worldProvider->canCoordinateBeSpawn(spawnX, spawnZ); ++attempts)
    {
		spawnX = JavaArithmetic::intAdd(spawnX, spawnRandom.nextIntDifference(64));
		spawnZ = JavaArithmetic::intAdd(spawnZ, spawnRandom.nextIntDifference(64));
    }
#endif

    worldInfo->setSpawn(spawnX, spawnY, spawnZ);
    findingSpawnPoint = false;
}

void World::getInitialSpawnLocation()
{
    generateSpawnPoint();
}

ChunkCoordinates *World::getEntrancePortalLocation()
{
    return worldProvider != nullptr ? worldProvider->getEntrancePortalLocation() : nullptr;
}

void World::setSpawnLocation()
{
    if (worldInfo->getSpawnY() <= 0)
    {
        worldInfo->setSpawnY(64);
    }
    
    int x = worldInfo->getSpawnX();
    int z = worldInfo->getSpawnZ();
    int_t attempts = 0;

    while (getFirstUncoveredBlock(x, z) == 0)
    {
        x = rand.nextIntOffset(x, 8);
        z = rand.nextIntOffset(z, 8);
        if (++attempts == 10000)
            break;
    }

#if PLATFORM_BOUNDED_WORLD
    worldInfo->setSpawnY(platformFindTopSpawnBlockY(this, x, z));
#endif
    
    worldInfo->setSpawnX(x);
    worldInfo->setSpawnZ(z);
}

int World::getFirstUncoveredBlock(int x, int z)
{
    int y = 63;
    while (!isAirBlock(x, y + 1, z))
    {
        y++;
    }
    return getBlockId(x, y, z);
}

void World::emptyMethod1()
{
}

void World::func_6464_c()
{
    emptyMethod1();
}

void World::spawnPlayerWithLoadedChunks(EntityPlayer* entityPlayer)
{
    try
    {
        NBTTagCompound* nbt = worldInfo->getPlayerNBTTagCompound();
        if (nbt != nullptr)
        {
            entityPlayer->readFromNBT(nbt);
            worldInfo->setPlayerNBTTagCompound(nullptr);
        }
        
        int chunkX = JavaArithmetic::intShr(MathHelper::floor_double(entityPlayer->posX), 4);
        int chunkZ = JavaArithmetic::intShr(MathHelper::floor_double(entityPlayer->posZ), 4);
        if (ChunkProviderLoadOrGenerate* chunkProviderLoadOrGenerate = dynamic_cast<ChunkProviderLoadOrGenerate*>(chunkProvider))
        {
            chunkProviderLoadOrGenerate->setCurrentChunkOver(chunkX, chunkZ);
        }
        else if (ChunkProvider* chunkProviderMap = dynamic_cast<ChunkProvider*>(chunkProvider))
        {
            chunkProviderMap->setCurrentChunkOver(chunkX, chunkZ);
        }
        
        entityJoinedWorld(entityPlayer);
    }
    catch (...)
    {
        // exception.printStackTrace();
    }
}

void World::saveWorld(bool flag, IProgressUpdate* progressUpdate)
{
    if (!chunkProvider->canSave())
    {
        return;
    }
    
    if (progressUpdate != nullptr)
    {
        progressUpdate->displaySavingString("Saving level");
    }
    
    MC_LOG_DEBUG("save", "[world save] saveLevel begin\n");
    saveLevel();
    MC_LOG_DEBUG("save", "[world save] saveLevel end\n");
    
    if (progressUpdate != nullptr)
    {
        progressUpdate->displayLoadingString("Saving chunks");
    }
    
    MC_LOG_DEBUG("save", "[world save] saveChunks begin full=%d\n", flag ? 1 : 0);
    const bool chunksDone = chunkProvider->saveChunks(flag, progressUpdate);
    MC_LOG_DEBUG("save", "[world save] saveChunks end full=%d done=%d\n",
                 flag ? 1 : 0, chunksDone ? 1 : 0);
}

void World::saveLevel()
{
    checkSessionLock();
#if PLATFORM_PROFILE_STREAMING && MC_LOG_LEVEL >= 2
    const long_t saveInfoStartNs = System::nanoTime();
#endif
    saveHandler->saveWorldInfoAndPlayer(worldInfo, playerEntities);
#if PLATFORM_PROFILE_STREAMING && MC_LOG_LEVEL >= 2
    platformProfileSaveWorldInfo(System::nanoTime() - saveInfoStartNs);
    const long_t mapStorageStartNs = System::nanoTime();
#endif
    mapStorage->saveAllData();
#if PLATFORM_PROFILE_STREAMING && MC_LOG_LEVEL >= 2
    platformProfileMapStorage(System::nanoTime() - mapStorageStartNs);
#endif
}

bool World::saveAllChunks(int i)
{
    if (!chunkProvider->canSave())
    {
        return true;
    }
    
    if (i == 0)
    {
        saveLevel();
    }
    
    return chunkProvider->saveChunks(false, nullptr);
}

bool World::quickSaveWorld(int_t progressStage)
{
    return saveAllChunks(progressStage);
}


Chunk *World::getPopulationFastChunk(int_t x, int_t y, int_t z) const
{
    if (!populationFastPathActive)
        return nullptr;
    return populationRegionAccessor.getChunk(x, y, z);
}

int World::getBlockId(int x, int y, int z)
{
    if (x < -30000000 || z < -30000000 || x >= 30000000 || z >= 30000000)
    {
        return 0;
    }
    
    if (y < 0)
    {
        return 0;
    }
    
    if (y >= WorldHeight::HEIGHT)
    {
        return 0;
    }

    if (chunkLocalDecoration.isActive())
        return chunkLocalDecoration.getBlockId(x, y, z);

#if PLATFORM_POPULATION_BLOCK_WRITER
    int_t populationBlockId = 0;
    if (populationRegionAccessor.tryGetBlockId(x, y, z, populationBlockId))
    {
        platformProfilePopulationAccess(PlatformPopulationAccessKind::BlockRead, true);
        return populationBlockId;
    }
#endif
    if (populationFastPathActive)
        platformProfilePopulationAccess(PlatformPopulationAccessKind::BlockRead, false);

    return getChunkFromChunkCoords(JavaArithmetic::intShr(x, 4), JavaArithmetic::intShr(z, 4))->getBlockID(x & 0xf, y, z & 0xf);
}

int_t World::getBlockLightOpacity(int_t x, int_t y, int_t z)
{
    if (x < -30000000 || z < -30000000 || x >= 30000000 || z >= 30000000 ||
        y < 0 || y >= WorldHeight::HEIGHT)
        return 0;

    Chunk *chunk = getChunkFromChunkCoords(JavaArithmetic::intShr(x, 4), JavaArithmetic::intShr(z, 4));
    return chunk != nullptr ? chunk->getBlockLightOpacity(x & 15, y, z & 15) : 0;
}


int_t World::func_48462_d(int_t x, int_t y, int_t z)
{
    return getBlockLightOpacity(x, y, z);
}

bool World::isAirBlock(int x, int y, int z)
{
    return getBlockId(x, y, z) == 0;
}

bool World::blockExists(int x, int y, int z)
{
    if (y < 0 || y >= WorldHeight::HEIGHT)
    {
        return false;
    }
    
    return chunkExists(JavaArithmetic::intShr(x, 4), JavaArithmetic::intShr(z, 4));
}

bool World::doChunksNearChunkExist(int x, int y, int z, int range)
{
    return checkChunksExist(JavaArithmetic::intSub(x, range), JavaArithmetic::intSub(y, range), JavaArithmetic::intSub(z, range),
        JavaArithmetic::intAdd(x, range), JavaArithmetic::intAdd(y, range), JavaArithmetic::intAdd(z, range));
}

bool World::checkChunksExist(int minX, int minY, int minZ, int maxX, int maxY, int maxZ)
{
    if (maxY < 0 || minY >= WorldHeight::HEIGHT)
    {
        return false;
    }
    
    minX = JavaArithmetic::intShr(minX, 4);
    minZ = JavaArithmetic::intShr(minZ, 4);
    maxX = JavaArithmetic::intShr(maxX, 4);
    maxZ = JavaArithmetic::intShr(maxZ, 4);
    
    for (int cx = minX; cx <= maxX; cx++)
    {
        for (int cz = minZ; cz <= maxZ; cz++)
        {
            if (!chunkExists(cx, cz))
            {
                return false;
            }
        }
    }
    
    return true;
}

bool World::chunkExists(int chunkX, int chunkZ)
{
    return chunkProvider->chunkExists(chunkX, chunkZ);
}

Chunk *World::getChunkIfExists(int_t chunkX, int_t chunkZ)
{
    return chunkProvider != nullptr ? chunkProvider->getChunkIfExists(chunkX, chunkZ) : nullptr;
}

bool World::isChunkPopulationPendingForRendering(int_t chunkX, int_t chunkZ) const
{
#if PLATFORM_DEFER_MESH_DURING_POPULATE
    if (chunkProvider == nullptr)
        return false;

    // Population rooted at any of these four chunks can write into (chunkX, chunkZ).
    return chunkProvider->isChunkPopulationPending(chunkX, chunkZ) ||
           chunkProvider->isChunkPopulationPending(chunkX - 1, chunkZ) ||
           chunkProvider->isChunkPopulationPending(chunkX, chunkZ - 1) ||
           chunkProvider->isChunkPopulationPending(chunkX - 1, chunkZ - 1);
#else
    (void)chunkX;
    (void)chunkZ;
    return false;
#endif
}

bool World::isChunkInLoadRadius(int chunkX, int chunkZ)
{
#if PLATFORM_BOUNDED_WORLD
    // Only the bounded ServerChunkCache knows its load radius; for other providers
    // (PC, client, nether/sky) a missing chunk is never a deferred-generation case.
    if (ChunkProvider* cp = dynamic_cast<ChunkProvider*>(chunkProvider))
        return cp->canChunkExist(chunkX, chunkZ);
#else
    (void)chunkX; (void)chunkZ;
#endif
    return false;
}

bool World::isChunkResident(int_t chunkX, int_t chunkZ) const
{
    const int_t radius = worldProvider != nullptr ? worldProvider->getResidentChunkRadius() : -1;
    return radius >= 0 &&
           chunkX >= -radius && chunkX <= radius &&
           chunkZ >= -radius && chunkZ <= radius;
}

bool World::isChunkRetainedByEntity(int_t chunkX, int_t chunkZ) const
{
#if PLATFORM_ENTITY_CHUNK_RETENTION
    for (Entity *entity : loadedEntityList)
    {
        if (entity == nullptr || entity->isDead)
            continue;

        const int_t radius = entity->getChunkRetentionRadius();
        if (radius < 0)
            continue;

        const int_t entityChunkX = MathHelper::floor_double(entity->posX / 16.0);
        const int_t entityChunkZ = MathHelper::floor_double(entity->posZ / 16.0);
        const long_t dx = static_cast<long_t>(chunkX) - static_cast<long_t>(entityChunkX);
        const long_t dz = static_cast<long_t>(chunkZ) - static_cast<long_t>(entityChunkZ);
        if (dx >= -radius && dx <= radius && dz >= -radius && dz <= radius)
            return true;
    }
#else
    (void)chunkX;
    (void)chunkZ;
#endif
    return false;
}

bool World::isChunkRequiredByRetainedEntity(int_t chunkX, int_t chunkZ) const
{
#if PLATFORM_ENTITY_CHUNK_RETENTION
    for (Entity *entity : loadedEntityList)
    {
        if (entity == nullptr || entity->isDead || entity->getChunkRetentionRadius() < 0)
            continue;

        const int_t entityChunkX = MathHelper::floor_double(entity->posX / 16.0);
        const int_t entityChunkZ = MathHelper::floor_double(entity->posZ / 16.0);
        if (chunkX == entityChunkX && chunkZ == entityChunkZ)
            return true;
    }
#else
    (void)chunkX;
    (void)chunkZ;
#endif
    return false;
}

Chunk* World::getChunkFromBlockCoords(int x, int z)
{
    return getChunkFromChunkCoords(JavaArithmetic::intShr(x, 4), JavaArithmetic::intShr(z, 4));
}

Chunk* World::getChunkFromChunkCoords(int chunkX, int chunkZ)
{
    return chunkProvider->provideChunk(chunkX, chunkZ);
}

bool World::setBlockAndMetadata(int x, int y, int z, int blockId, int metadata)
{
    if (x < -30000000 || z < -30000000 || x >= 30000000 || z >= 30000000)
    {
        return false;
    }
    
    if (y < 0)
    {
        return false;
    }
    
    if (y >= WorldHeight::HEIGHT)
    {
        return false;
    }

    if (chunkLocalDecoration.isActive())
        return chunkLocalDecoration.setBlock(x, y, z, blockId, metadata);

#if PLATFORM_POPULATION_BLOCK_WRITER
    Chunk *populationChunk = getPopulationFastChunk(x, y, z);
    if (populationChunk != nullptr)
    {
        platformProfilePopulationAccess(PlatformPopulationAccessKind::BlockWrite, true);
        return populationChunk->setBlockIDWithMetadata(x & 0xf, y, z & 0xf, blockId, metadata);
    }
#endif
    if (populationFastPathActive)
        platformProfilePopulationAccess(PlatformPopulationAccessKind::BlockWrite, false);

    Chunk* chunk = getChunkFromChunkCoords(JavaArithmetic::intShr(x, 4), JavaArithmetic::intShr(z, 4));
    return chunk->setBlockIDWithMetadata(x & 0xf, y, z & 0xf, blockId, metadata);
}

bool World::setBlock(int x, int y, int z, int blockId)
{
    if (x < -30000000 || z < -30000000 || x >= 30000000 || z >= 30000000)
    {
        return false;
    }
    
    if (y < 0)
    {
        return false;
    }
    
    if (y >= WorldHeight::HEIGHT)
    {
        return false;
    }

    if (chunkLocalDecoration.isActive())
        return chunkLocalDecoration.setBlock(x, y, z, blockId, 0);

#if PLATFORM_POPULATION_BLOCK_WRITER
    Chunk *populationChunk = getPopulationFastChunk(x, y, z);
    if (populationChunk != nullptr)
    {
        platformProfilePopulationAccess(PlatformPopulationAccessKind::BlockWrite, true);
        return populationChunk->setBlockID(x & 0xf, y, z & 0xf, blockId);
    }
#endif
    if (populationFastPathActive)
        platformProfilePopulationAccess(PlatformPopulationAccessKind::BlockWrite, false);

    Chunk* chunk = getChunkFromChunkCoords(JavaArithmetic::intShr(x, 4), JavaArithmetic::intShr(z, 4));
    return chunk->setBlockID(x & 0xf, y, z & 0xf, blockId);
}

void World::beginPopulationFastPath(int_t chunkX, int_t chunkZ)
{
    endPopulationFastPath();
    populationFastBaseX = chunkX;
    populationFastBaseZ = chunkZ;

    for (int_t dz = 0; dz < 2; ++dz)
    {
        for (int_t dx = 0; dx < 2; ++dx)
        {
            const int_t cx = chunkX + dx;
            const int_t cz = chunkZ + dz;
            if (!chunkExists(cx, cz))
            {
                endPopulationFastPath();
                return;
            }

            Chunk *chunk = getChunkFromChunkCoords(cx, cz);
            if (chunk == nullptr || chunk->isEmptyChunk())
            {
                endPopulationFastPath();
                return;
            }
            populationFastChunks[dz * 2 + dx] = chunk;
        }
    }

    populationRegionAccessor.bind(populationFastBaseX, populationFastBaseZ, populationFastChunks);
    populationFastPathActive = true;
}

void World::beginChunkLocalDecoration(int_t chunkX, int_t chunkZ, byte_t *blocks, byte_t *metadata,
                                      const std::vector<StructureBoundingBox> *structureBounds)
{
    chunkLocalDecoration.bind(chunkX, chunkZ, blocks, metadata, structureBounds);
}

void World::endChunkLocalDecoration()
{
    chunkLocalDecoration.reset();
}

void World::endPopulationFastPath()
{
#if PLATFORM_PS2 && MC_LOG_LEVEL > 2
    const bool profileClose = populationFastPathActive;
    const std::uint32_t lightingStart = profileClose ? platformProfileRenderPhaseBegin() : 0;
#endif
    populationFastPathActive = false;
    populationRegionAccessor.reset();
    flushPopulationLightingBatches();
#if PLATFORM_PS2 && MC_LOG_LEVEL > 2
    if (profileClose)
        platformProfilePopulationFastPathStage(lightingStart, PlatformPopulationFastPathStage::LightingFlush, 1);
    const std::uint32_t skylightStart = profileClose ? platformProfileRenderPhaseBegin() : 0;
    int skylightRegens = 0;
#endif
    for (int_t index = 0; index < 4; ++index)
    {
        Chunk *chunk = populationFastChunks[index];
        if (chunk != nullptr && chunk->skylightRegenPending)
        {
            const int_t dirtyColumns = chunk->flushPopulationSkylightColumns();
#if PLATFORM_PS2 && MC_LOG_LEVEL > 2
            if (dirtyColumns > 0)
                ++skylightRegens;
#else
            (void)dirtyColumns;
#endif
        }
        populationFastChunks[index] = nullptr;
    }
#if PLATFORM_PS2 && MC_LOG_LEVEL > 2
    if (profileClose)
        platformProfilePopulationFastPathStage(skylightStart, PlatformPopulationFastPathStage::SkylightRegen, skylightRegens);
#endif
}

bool World::isPopulationFastPathChunk(const Chunk *chunk) const
{
    if (!populationFastPathActive || chunk == nullptr)
        return false;
    for (int_t index = 0; index < 4; ++index)
    {
        if (populationFastChunks[index] == chunk)
            return true;
    }
    return false;
}

int_t World::getBlockIdForPopulation(int_t x, int_t y, int_t z)
{
    return getBlockId(x, y, z);
}


Chunk *World::getChunkForPopulation(int_t x, int_t y, int_t z)
{
    // No Chunk exists yet; WorldGenerator::setBlockAndMetadata then falls
    // through to setBlockAndMetadataForPopulation, which writes the buffer.
    if (chunkLocalDecoration.isActive())
        return nullptr;
#if PLATFORM_POPULATION_BLOCK_WRITER
    Chunk *populationChunk = getPopulationFastChunk(x, y, z);
    if (populationChunk != nullptr)
    {
        platformProfilePopulationAccess(PlatformPopulationAccessKind::ChunkLookup, true);
        return populationChunk;
    }
#endif
    if (populationFastPathActive)
        platformProfilePopulationAccess(PlatformPopulationAccessKind::ChunkLookup, false);

    if (!blockExists(x, y, z))
        return nullptr;
    return getChunkFromBlockCoords(x, z);
}


bool World::setBlockAndMetadataForPopulation(int_t x, int_t y, int_t z, int_t blockId, int_t metadata)
{
    if (chunkLocalDecoration.isActive())
        return chunkLocalDecoration.setBlock(x, y, z, blockId, metadata);
    platformProfilePopulationBlockWrite();
#if PLATFORM_POPULATION_BLOCK_WRITER
    Chunk *populationChunk = getPopulationFastChunk(x, y, z);
    if (populationChunk != nullptr &&
        (blockId == Block::leaves->blockID || blockId == Block::wood->blockID ||
         blockId == Block::vine->blockID || blockId == Block::tallGrass->blockID))
    {
        platformProfilePopulationAccess(PlatformPopulationAccessKind::BlockWrite, true);
        return populationChunk->setVegetationBlockIDWithMetadataForPopulation(
            x & 0xf, y, z & 0xf, blockId, metadata);
    }
#endif
    return setBlockAndMetadata(x, y, z, blockId, metadata);
}


bool World::batchPopulationLightingUpdate(EnumSkyBlock *type, int_t minX, int_t minY, int_t minZ,
                                          int_t maxX, int_t maxY, int_t maxZ)
{
#if PLATFORM_BATCH_POPULATION_LIGHTING
    if (!populationFastPathActive || type == nullptr || minX > maxX || minY > maxY || minZ > maxZ)
        return false;

    const int_t typeIndex = type == EnumSkyBlock::Sky ? 0 : (type == EnumSkyBlock::Block ? 1 : -1);
    if (typeIndex < 0)
        return false;

    if (maxY < WorldHeight::MIN_Y || minY >= WorldHeight::HEIGHT)
        return false;
    minY = std::max<int_t>(minY, WorldHeight::MIN_Y);
    maxY = std::min<int_t>(maxY, WorldHeight::MAX_Y);

    const int_t minChunkX = JavaArithmetic::intShr(minX, 4);
    const int_t maxChunkX = JavaArithmetic::intShr(maxX, 4);
    const int_t minChunkZ = JavaArithmetic::intShr(minZ, 4);
    const int_t maxChunkZ = JavaArithmetic::intShr(maxZ, 4);
    if (minChunkX < populationFastBaseX || maxChunkX > populationFastBaseX + 1 ||
        minChunkZ < populationFastBaseZ || maxChunkZ > populationFastBaseZ + 1)
        return false;

    const int_t minSection = minY >> 4;
    const int_t maxSection = maxY >> 4;
    for (int_t chunkZ = minChunkZ; chunkZ <= maxChunkZ; ++chunkZ)
    {
        for (int_t chunkX = minChunkX; chunkX <= maxChunkX; ++chunkX)
        {
            const int_t dx = chunkX - populationFastBaseX;
            const int_t dz = chunkZ - populationFastBaseZ;
            const int_t chunkIndex = dz * 2 + dx;
            const int_t chunkMinX = JavaArithmetic::intMul(chunkX, 16);
            const int_t chunkMinZ = JavaArithmetic::intMul(chunkZ, 16);
            const int_t clippedMinX = std::max<int_t>(minX, chunkMinX);
            const int_t clippedMaxX = std::min<int_t>(maxX, chunkMinX + 15);
            const int_t clippedMinZ = std::max<int_t>(minZ, chunkMinZ);
            const int_t clippedMaxZ = std::min<int_t>(maxZ, chunkMinZ + 15);

            for (int_t section = minSection; section <= maxSection; ++section)
            {
                const int_t sectionMinY = section << 4;
                const int_t clippedMinY = std::max<int_t>(minY, sectionMinY);
                const int_t clippedMaxY = std::min<int_t>(maxY, sectionMinY + 15);
                const int_t batchIndex =
                    (typeIndex * POPULATION_LIGHTING_CHUNK_COUNT + chunkIndex) *
                    WorldHeight::SECTION_COUNT + section;
                PopulationLightingBatch &batch = populationLightingBatches[batchIndex];
                if (!batch.valid)
                {
                    batch.valid = true;
                    batch.minX = clippedMinX;
                    batch.minY = clippedMinY;
                    batch.minZ = clippedMinZ;
                    batch.maxX = clippedMaxX;
                    batch.maxY = clippedMaxY;
                    batch.maxZ = clippedMaxZ;
                }
                else
                {
                    batch.minX = std::min(batch.minX, clippedMinX);
                    batch.minY = std::min(batch.minY, clippedMinY);
                    batch.minZ = std::min(batch.minZ, clippedMinZ);
                    batch.maxX = std::max(batch.maxX, clippedMaxX);
                    batch.maxY = std::max(batch.maxY, clippedMaxY);
                    batch.maxZ = std::max(batch.maxZ, clippedMaxZ);
                }
            }
        }
    }
    return true;
#else
    (void)type;
    (void)minX;
    (void)minY;
    (void)minZ;
    (void)maxX;
    (void)maxY;
    (void)maxZ;
    return false;
#endif
}

void World::flushPopulationLightingBatches()
{
#if PLATFORM_BATCH_POPULATION_LIGHTING
    constexpr int_t batchesPerType =
        POPULATION_LIGHTING_CHUNK_COUNT * WorldHeight::SECTION_COUNT;
    for (int_t typeIndex = 0; typeIndex < POPULATION_LIGHTING_TYPE_COUNT; ++typeIndex)
    {
        EnumSkyBlock *type = typeIndex == 0 ? EnumSkyBlock::Sky : EnumSkyBlock::Block;
        const int_t firstBatch = typeIndex * batchesPerType;
        const int_t lastBatch = firstBatch + batchesPerType;
        for (int_t batchIndex = firstBatch; batchIndex < lastBatch; ++batchIndex)
        {
            PopulationLightingBatch &batch = populationLightingBatches[batchIndex];
            if (!batch.valid)
                continue;

            const int_t minX = batch.minX;
            const int_t minY = batch.minY;
            const int_t minZ = batch.minZ;
            const int_t maxX = batch.maxX;
            const int_t maxY = batch.maxY;
            const int_t maxZ = batch.maxZ;
            batch.valid = false;

            // Each batch is clipped to one 16x16x16 chunk section, keeping the
            // flood-fill volume well below MetadataChunkBlock's 32768-cell cap.
            scheduleLightingUpdate_do(type, minX, minY, minZ, maxX, maxY, maxZ, true);
        }
    }
#else
    for (PopulationLightingBatch &batch : populationLightingBatches)
        batch.valid = false;
#endif
}

bool World::replaceBlockForPopulation(int_t x, int_t y, int_t z,
	                                  int_t expectedId, int_t newId)
{
    if (y < 0 || y >= WorldHeight::HEIGHT || expectedId < 0 || expectedId >= Block::BLOCK_REGISTRY_SIZE ||
        newId < 0 || newId >= Block::BLOCK_REGISTRY_SIZE || expectedId == newId)
        return false;

    if (chunkLocalDecoration.isActive())
    {
        if (chunkLocalDecoration.getBlockId(x, y, z) != expectedId)
            return false;
        return chunkLocalDecoration.setBlock(x, y, z, newId, 0);
    }

    // This shortcut is valid only when neither the light field nor tile-entity
    // ownership can change. All other cases retain vanilla setBlock semantics.
    const bool lightEquivalent =
        Block::lightOpacity[expectedId] == Block::lightOpacity[newId] &&
        Block::lightValue[expectedId] == Block::lightValue[newId];
    const bool simpleBlocks =
        !Block::isBlockContainer[expectedId] && !Block::isBlockContainer[newId];
	const bool callbackFreeReplacement =
		(expectedId == Block::stone->blockID &&
		 (newId == Block::dirt->blockID || newId == Block::oreCoal->blockID ||
		  newId == Block::oreIron->blockID || newId == Block::oreGold->blockID ||
		  newId == Block::oreRedstone->blockID || newId == Block::oreDiamond->blockID ||
		  newId == Block::oreLapis->blockID)) ||
		(expectedId == Block::sand->blockID && newId == Block::blockClay->blockID) ||
		(expectedId == Block::dirt->blockID && newId == Block::blockClay->blockID);
	const bool fallingBlockReplacement =
		(expectedId == Block::stone->blockID && newId == Block::gravel->blockID) ||
		((expectedId == Block::dirt->blockID || expectedId == Block::grass->blockID) &&
		 (newId == Block::sand->blockID || newId == Block::gravel->blockID));
	const bool directReplacement = callbackFreeReplacement || fallingBlockReplacement;

    if (populationFastPathActive && lightEquivalent && simpleBlocks && directReplacement)
    {
        const int_t cx = JavaArithmetic::intShr(x, 4);
        const int_t cz = JavaArithmetic::intShr(z, 4);
        const int_t dx = cx - populationFastBaseX;
        const int_t dz = cz - populationFastBaseZ;
        if (dx >= 0 && dx < 2 && dz >= 0 && dz < 2)
        {
            Chunk *chunk = populationFastChunks[dz * 2 + dx];
            if (chunk != nullptr)
            {
                const bool changed = chunk->replaceBlockIDForPopulation(
                    x & 0xf, y, z & 0xf, expectedId, newId);
                if (changed && fallingBlockReplacement && Block::blocksList[newId] != nullptr)
                    Block::blocksList[newId]->onBlockAdded(this, x, y, z);
                return changed;
            }
        }
    }

    if (getBlockId(x, y, z) != expectedId)
        return false;
    return setBlock(x, y, z, newId);
}

Material* World::getBlockMaterial(int x, int y, int z)
{
    int blockId = getBlockId(x, y, z);
    if (blockId == 0)
    {
        return Material::air;
    }
    
    return Block::blocksList[blockId]->blockMaterial;
}

int World::getBlockMetadata(int x, int y, int z)
{
    if (x < -30000000 || z < -30000000 || x >= 30000000 || z >= 30000000)
    {
        return 0;
    }
    
    if (y < 0)
    {
        return 0;
    }
    
    if (y >= WorldHeight::HEIGHT)
    {
        return 0;
    }

    if (chunkLocalDecoration.isActive())
        return chunkLocalDecoration.getBlockMetadata(x, y, z);

    Chunk* chunk = getChunkFromChunkCoords(JavaArithmetic::intShr(x, 4), JavaArithmetic::intShr(z, 4));
    x &= 0xf;
    z &= 0xf;
    return chunk->getBlockMetadata(x, y, z);
}

void World::setBlockMetadataWithNotify(int x, int y, int z, int metadata)
{
    if (setBlockMetadata(x, y, z, metadata))
    {
        int blockId = getBlockId(x, y, z);
        if (blockId >= 0 && blockId < Block::BLOCK_REGISTRY_SIZE &&
            Block::requiresSelfNotify[blockId])
        {
            notifyBlockChange(x, y, z, blockId);
        }
        else
        {
            notifyBlocksOfNeighborChange(x, y, z, blockId);
        }
    }
}

bool World::setBlockMetadata(int x, int y, int z, int metadata)
{
    if (x < -30000000 || z < -30000000 || x >= 30000000 || z >= 30000000)
    {
        return false;
    }
    
    if (y < 0)
    {
        return false;
    }
    
    if (y >= WorldHeight::HEIGHT)
    {
        return false;
    }

    if (chunkLocalDecoration.isActive())
    {
        return chunkLocalDecoration.setBlock(x, y, z,
            chunkLocalDecoration.getBlockId(x, y, z), metadata);
    }

    Chunk* chunk = getChunkFromChunkCoords(JavaArithmetic::intShr(x, 4), JavaArithmetic::intShr(z, 4));
    x &= 0xf;
    z &= 0xf;
    chunk->setBlockMetadata(x, y, z, metadata);
    return true;
}

bool World::setBlockWithNotify(int x, int y, int z, int blockId)
{
    if (setBlock(x, y, z, blockId))
    {
        if (!chunkLocalDecoration.isActive())
            notifyBlockChange(x, y, z, blockId);
        return true;
    }
    
    return false;
}

bool World::setBlockAndMetadataWithNotify(int x, int y, int z, int blockId, int metadata)
{
    if (setBlockAndMetadata(x, y, z, blockId, metadata))
    {
        if (!chunkLocalDecoration.isActive())
            notifyBlockChange(x, y, z, blockId);
        return true;
    }
    
    return false;
}

void World::markBlockNeedsUpdate(int x, int y, int z)
{
    for (size_t i = 0; i < worldAccesses.size(); i++)
    {
        worldAccesses[i]->markBlockAndNeighborsNeedsUpdate(x, y, z);
    }
}

void World::notifyBlockChange(int x, int y, int z, int blockId)
{
    // A buffer under chunk-local decoration has no renderer and no live
    // neighbours to react; the chunk gets one light/mesh pass when published.
    if (chunkLocalDecoration.isActive())
        return;
    markBlockNeedsUpdate(x, y, z);
    notifyBlocksOfNeighborChange(x, y, z, blockId);
}

void World::markBlocksDirtyVertical(int x, int z, int y1, int y2)
{
    if (y1 > y2)
    {
        int temp = y1;
        y1 = y2;
        y2 = temp;
    }
    
    markBlocksDirty(x, y1, z, x, y2, z);
}

void World::markBlockAsNeedsUpdate(int x, int y, int z)
{
    for (size_t i = 0; i < worldAccesses.size(); i++)
    {
        worldAccesses[i]->markBlockRangeNeedsUpdate(x, y, z, x, y, z);
    }
}

void World::markBlocksDirty(int minX, int minY, int minZ, int maxX, int maxY, int maxZ)
{
    for (size_t i = 0; i < worldAccesses.size(); i++)
    {
        worldAccesses[i]->markBlockRangeNeedsUpdate(minX, minY, minZ, maxX, maxY, maxZ);
    }
}

void World::notifyChunkPublishedForRender(int_t chunkX, int_t chunkZ)
{
    for (size_t i = 0; i < worldAccesses.size(); ++i)
        worldAccesses[i]->onChunkPublished(chunkX, chunkZ);
}

void World::notifyBlocksOfNeighborChange(int x, int y, int z, int blockId)
{
    if (chunkLocalDecoration.isActive())
        return;
    notifyBlockOfNeighborChange(x - 1, y, z, blockId);
    notifyBlockOfNeighborChange(x + 1, y, z, blockId);
    notifyBlockOfNeighborChange(x, y - 1, z, blockId);
    notifyBlockOfNeighborChange(x, y + 1, z, blockId);
    notifyBlockOfNeighborChange(x, y, z - 1, blockId);
    notifyBlockOfNeighborChange(x, y, z + 1, blockId);
}

void World::notifyBlockOfNeighborChange(int x, int y, int z, int blockId)
{
    if (editingBlocks || multiplayerWorld)
    {
        return;
    }
    
    Block* block = Block::blocksList[getBlockId(x, y, z)];
    if (block != nullptr)
    {
        block->onNeighborBlockChange(this, x, y, z, blockId);
    }
}

bool World::canBlockSeeTheSky(int x, int y, int z)
{
    if (chunkLocalDecoration.isActive())
        return y >= chunkLocalDecoration.getHeightValue(x, z);
#if PLATFORM_POPULATION_BLOCK_WRITER
    Chunk *populationChunk = getPopulationFastChunk(x, y, z);
    if (populationChunk != nullptr)
    {
        platformProfilePopulationAccess(PlatformPopulationAccessKind::ChunkLookup, true);
        return populationChunk->canBlockSeeTheSky(x & 0xf, y, z & 0xf);
    }
#endif
    if (populationFastPathActive)
        platformProfilePopulationAccess(PlatformPopulationAccessKind::ChunkLookup, false);
    return getChunkFromChunkCoords(JavaArithmetic::intShr(x, 4), JavaArithmetic::intShr(z, 4))->canBlockSeeTheSky(x & 0xf, y, z & 0xf);
}

int World::getFullBlockLightValue(int x, int y, int z)
{
    // Unlit buffer: skylight is full above the column's height map and dark
    // below, which is what the finished chunk's initial skylight will say.
    if (chunkLocalDecoration.isActive())
        return y >= chunkLocalDecoration.getHeightValue(x, z) ? 15 : 0;

    if (y < 0)
    {
        return 0;
    }
    
    if (y >= WorldHeight::HEIGHT)
    {
        y = WorldHeight::MAX_Y;
    }

#if PLATFORM_POPULATION_BLOCK_WRITER
    Chunk *populationChunk = getPopulationFastChunk(x, y, z);
    if (populationChunk != nullptr)
    {
        platformProfilePopulationAccess(PlatformPopulationAccessKind::ChunkLookup, true);
        return populationChunk->getBlockLightValue(x & 0xf, y, z & 0xf, 0);
    }
#endif
    if (populationFastPathActive)
        platformProfilePopulationAccess(PlatformPopulationAccessKind::ChunkLookup, false);

    return getChunkFromChunkCoords(JavaArithmetic::intShr(x, 4), JavaArithmetic::intShr(z, 4))->getBlockLightValue(x & 0xf, y, z & 0xf, 0);
}

int World::getBlockLightValue(int x, int y, int z)
{
    return getBlockLightValue_do(x, y, z, true);
}

int World::getBlockLightValue_do(int x, int y, int z, bool flag)
{
    if (x < -30000000 || z < -30000000 || x >= 30000000 || z >= 30000000)
    {
        return 15;
    }

    if (chunkLocalDecoration.isActive())
        return y >= chunkLocalDecoration.getHeightValue(x, z) ? 15 : 0;
    
    if (flag)
    {
        int blockId = getBlockId(x, y, z);
        if (blockId == Block::stairSingle->blockID || 
            blockId == Block::tilledField->blockID ||
            blockId == Block::stairCompactCobblestone->blockID ||
            blockId == Block::stairCompactPlanks->blockID)
        {
            int lightValue = getBlockLightValue_do(x, y + 1, z, false);
            lightValue = std::max(lightValue, getBlockLightValue_do(x + 1, y, z, false));
            lightValue = std::max(lightValue, getBlockLightValue_do(x - 1, y, z, false));
            lightValue = std::max(lightValue, getBlockLightValue_do(x, y, z + 1, false));
            lightValue = std::max(lightValue, getBlockLightValue_do(x, y, z - 1, false));
            return lightValue;
        }
    }
    
    if (y < 0)
    {
        return 0;
    }
    
    if (y >= WorldHeight::HEIGHT)
    {
        y = WorldHeight::MAX_Y;
    }
    
    Chunk* chunk = nullptr;
#if PLATFORM_POPULATION_BLOCK_WRITER
    chunk = getPopulationFastChunk(x, y, z);
    if (chunk != nullptr)
        platformProfilePopulationAccess(PlatformPopulationAccessKind::ChunkLookup, true);
#endif
    if (chunk == nullptr)
    {
        if (populationFastPathActive)
            platformProfilePopulationAccess(PlatformPopulationAccessKind::ChunkLookup, false);
        chunk = getChunkFromChunkCoords(JavaArithmetic::intShr(x, 4), JavaArithmetic::intShr(z, 4));
    }
    x &= 0xf;
    z &= 0xf;
    return chunk->getBlockLightValue(x, y, z, skylightSubtracted);
}

bool World::canExistingBlockSeeTheSky(int x, int y, int z)
{
    if (x < -30000000 || z < -30000000 || x >= 30000000 || z >= 30000000)
    {
        return false;
    }
    
    if (y < 0)
    {
        return false;
    }
    
    if (y >= WorldHeight::HEIGHT)
    {
        return true;
    }
    
    if (!chunkExists(JavaArithmetic::intShr(x, 4), JavaArithmetic::intShr(z, 4)))
    {
        return false;
    }
    
    Chunk* chunk = getChunkFromChunkCoords(JavaArithmetic::intShr(x, 4), JavaArithmetic::intShr(z, 4));
    x &= 0xf;
    z &= 0xf;
    return chunk->canBlockSeeTheSky(x, y, z);
}

int World::getHeightValue(int x, int z)
{
    if (x < -30000000 || z < -30000000 || x >= 30000000 || z >= 30000000)
    {
        return 0;
    }

    if (chunkLocalDecoration.isActive())
        return chunkLocalDecoration.getHeightValue(x, z);

#if PLATFORM_POPULATION_BLOCK_WRITER
    Chunk *populationChunk = getPopulationFastChunk(x, WorldHeight::MIN_Y, z);
    if (populationChunk != nullptr)
    {
        platformProfilePopulationAccess(PlatformPopulationAccessKind::ChunkLookup, true);
        return populationChunk->getHeightValue(x & 0xf, z & 0xf);
    }
#endif
    if (populationFastPathActive)
        platformProfilePopulationAccess(PlatformPopulationAccessKind::ChunkLookup, false);
    
    if (!chunkExists(JavaArithmetic::intShr(x, 4), JavaArithmetic::intShr(z, 4)))
    {
        return 0;
    }
    
    Chunk* chunk = getChunkFromChunkCoords(JavaArithmetic::intShr(x, 4), JavaArithmetic::intShr(z, 4));
    return chunk->getHeightValue(x & 0xf, z & 0xf);
}

int_t World::getPrecipitationHeight(int_t x, int_t z)
{
    Chunk *chunk = getChunkFromBlockCoords(x, z);
    return chunk != nullptr ? chunk->getPrecipitationHeight(x & 15, z & 15) : 0;
}

int_t World::getTopSolidOrLiquidBlock(int_t x, int_t z)
{
    if (chunkLocalDecoration.isActive())
        return chunkLocalDecoration.getTopSolidOrLiquidBlock(x, z);

    Chunk *chunk = nullptr;
#if PLATFORM_POPULATION_BLOCK_WRITER
    chunk = getPopulationFastChunk(x, WorldHeight::MIN_Y, z);
    if (chunk != nullptr)
        platformProfilePopulationAccess(PlatformPopulationAccessKind::ChunkLookup, true);
#endif
    if (chunk == nullptr)
    {
        if (populationFastPathActive)
            platformProfilePopulationAccess(PlatformPopulationAccessKind::ChunkLookup, false);
        chunk = getChunkFromBlockCoords(x, z);
    }
    if (chunk == nullptr)
        return -1;

    int_t y = chunk->getTopFilledSegment() + WorldHeight::SECTION_HEIGHT;
    const int_t localX = x & 15;
    const int_t localZ = z & 15;

    if (y >= WorldHeight::HEIGHT)
        y = WorldHeight::MAX_Y;

    for (; y > 0; --y)
    {
        const int_t blockId = chunk->getBlockID(localX, y, localZ);
        if (blockId <= 0 || blockId >= Block::BLOCK_REGISTRY_SIZE)
            continue;

        Block *block = Block::blocksList[blockId];
        if (block != nullptr && block->blockMaterial->getIsSolid() && block->blockMaterial != Material::leaves)
            return y + 1;
    }

    return -1;
}

BiomeGenBase *World::getBiomeGenForCoords(int_t x, int_t z)
{
    WorldChunkManager *manager = getWorldChunkManager();
    if (blockExists(x, 0, z))
    {
        Chunk *chunk = getChunkFromBlockCoords(x, z);
        if (chunk != nullptr)
            return chunk->func_48490_a(x & 15, z & 15, manager);
    }

    return manager != nullptr ? manager->getBiomeGenAt(x, z) : BiomeGenBase::plains;
}

bool World::isBlockHydratedDirectly(int_t x, int_t y, int_t z)
{
    return isBlockHydrated(x, y, z, false);
}

bool World::isBlockHydratedIndirectly(int_t x, int_t y, int_t z)
{
    return isBlockHydrated(x, y, z, true);
}

bool World::isBlockHydrated(int_t x, int_t y, int_t z, bool requireEdge)
{
    BiomeGenBase *biome = getBiomeGenForCoords(x, z);
    if (biome == nullptr || biome->getFloatTemperature() > 0.15f)
        return false;

    if (y < WorldHeight::MIN_Y || y >= WorldHeight::HEIGHT ||
        getSavedLightValue(EnumSkyBlock::Block, x, y, z) >= 10)
        return false;

    const int_t blockId = getBlockId(x, y, z);
    if ((blockId != Block::waterStill->blockID && blockId != Block::waterMoving->blockID) ||
        getBlockMetadata(x, y, z) != 0)
        return false;

    if (!requireEdge)
        return true;

    bool surroundedByWater = true;
    if (getBlockMaterial(x - 1, y, z) != Material::water) surroundedByWater = false;
    if (getBlockMaterial(x + 1, y, z) != Material::water) surroundedByWater = false;
    if (getBlockMaterial(x, y, z - 1) != Material::water) surroundedByWater = false;
    if (getBlockMaterial(x, y, z + 1) != Material::water) surroundedByWater = false;
    return !surroundedByWater;
}

bool World::canSnowAt(int_t x, int_t y, int_t z)
{
    BiomeGenBase *biome = getBiomeGenForCoords(x, z);
    if (biome == nullptr || biome->getFloatTemperature() > 0.15f)
        return false;

    if (y < WorldHeight::MIN_Y || y >= WorldHeight::HEIGHT ||
        getSavedLightValue(EnumSkyBlock::Block, x, y, z) >= 10)
        return false;

    const int_t blockBelow = getBlockId(x, y - 1, z);
    const int_t blockHere = getBlockId(x, y, z);
    return blockHere == 0 &&
           Block::snow->canPlaceBlockAt(this, x, y, z) &&
           blockBelow != 0 && blockBelow != Block::ice->blockID &&
           blockBelow < Block::BLOCK_REGISTRY_SIZE && Block::blocksList[blockBelow] != nullptr &&
           Block::blocksList[blockBelow]->blockMaterial->getIsSolid();
}

void World::neighborLightPropagationChanged(EnumSkyBlock* enumSkyBlock, int x, int y, int z, int lightValue)
{
    if (worldProvider->hasNoSky && enumSkyBlock == EnumSkyBlock::Sky)
    {
        return;
    }
    
    if (!blockExists(x, y, z))
    {
        return;
    }
    
    if (enumSkyBlock == EnumSkyBlock::Sky)
    {
        if (canExistingBlockSeeTheSky(x, y, z))
        {
            lightValue = 15;
        }
    }
    else if (enumSkyBlock == EnumSkyBlock::Block)
    {
        int blockId = getBlockId(x, y, z);
        if (Block::lightValue[blockId] > lightValue)
        {
            lightValue = Block::lightValue[blockId];
        }
    }
    
    if (getSavedLightValue(enumSkyBlock, x, y, z) != lightValue)
    {
        scheduleLightingUpdate(enumSkyBlock, x, y, z, x, y, z);
    }
}

int_t World::getSkyBlockTypeBrightness(EnumSkyBlock *enumSkyBlock, int_t x, int_t y, int_t z)
{
    if (enumSkyBlock == nullptr)
        return 0;
    if (worldProvider != nullptr && worldProvider->hasNoSky && enumSkyBlock == EnumSkyBlock::Sky)
        return 0;

    if (y < 0)
        y = 0;
    if (y >= WorldHeight::HEIGHT)
        return enumSkyBlock->defaultLightValue;
    if (x < -30000000 || z < -30000000 || x >= 30000000 || z >= 30000000)
        return enumSkyBlock->defaultLightValue;

    const int_t chunkX = JavaArithmetic::intShr(x, 4);
    const int_t chunkZ = JavaArithmetic::intShr(z, 4);
    if (!chunkExists(chunkX, chunkZ))
        return enumSkyBlock->defaultLightValue;

    const int_t blockId = getBlockId(x, y, z);
    if (blockId >= 0 && blockId < Block::BLOCK_REGISTRY_SIZE && Block::useNeighborBrightness[blockId])
    {
        int_t brightness = getSavedLightValue(enumSkyBlock, x, y + 1, z);
        brightness = std::max(brightness, getSavedLightValue(enumSkyBlock, x + 1, y, z));
        brightness = std::max(brightness, getSavedLightValue(enumSkyBlock, x - 1, y, z));
        brightness = std::max(brightness, getSavedLightValue(enumSkyBlock, x, y, z + 1));
        brightness = std::max(brightness, getSavedLightValue(enumSkyBlock, x, y, z - 1));
        return brightness;
    }

    Chunk *chunk = getChunkFromChunkCoords(chunkX, chunkZ);
    return chunk != nullptr
        ? chunk->getSavedLightValue(enumSkyBlock, x & 0xf, y, z & 0xf)
        : enumSkyBlock->defaultLightValue;
}

int World::getSavedLightValue(EnumSkyBlock* enumSkyBlock, int x, int y, int z)
{
    if (enumSkyBlock == nullptr)
        return 0;
    if (y < 0)
        y = 0;
    if (y >= WorldHeight::HEIGHT)
        y = WorldHeight::MAX_Y;
    if (x < -30000000 || z < -30000000 || x >= 30000000 || z >= 30000000)
        return enumSkyBlock->defaultLightValue;

    const int_t chunkX = JavaArithmetic::intShr(x, 4);
    const int_t chunkZ = JavaArithmetic::intShr(z, 4);
    if (!chunkExists(chunkX, chunkZ))
        return enumSkyBlock->defaultLightValue;

    Chunk* chunk = getChunkFromChunkCoords(chunkX, chunkZ);
    return chunk != nullptr
        ? chunk->getSavedLightValue(enumSkyBlock, x & 0xf, y, z & 0xf)
        : enumSkyBlock->defaultLightValue;
}

int_t World::getLightBrightnessForSkyBlocks(int_t x, int_t y, int_t z, int_t minimumBlockLight)
{
    int_t skyLight = getSkyBlockTypeBrightness(EnumSkyBlock::Sky, x, y, z);
    int_t blockLight = getSkyBlockTypeBrightness(EnumSkyBlock::Block, x, y, z);
    if (blockLight < minimumBlockLight)
        blockLight = minimumBlockLight;
    return (skyLight << 20) | (blockLight << 4);
}

void World::setLightValue(EnumSkyBlock* enumSkyBlock, int x, int y, int z, int lightValue)
{
    if (x < -30000000 || z < -30000000 || x >= 30000000 || z >= 30000000)
    {
        return;
    }
    
    if (y < 0)
    {
        return;
    }
    
    if (y >= WorldHeight::HEIGHT)
    {
        return;
    }
    
    if (!chunkExists(JavaArithmetic::intShr(x, 4), JavaArithmetic::intShr(z, 4)))
    {
        return;
    }
    
    Chunk* chunk = getChunkFromChunkCoords(JavaArithmetic::intShr(x, 4), JavaArithmetic::intShr(z, 4));
    chunk->setLightValue(enumSkyBlock, x & 0xf, y, z & 0xf, lightValue);
    
    for (size_t i = 0; i < worldAccesses.size(); i++)
    {
        worldAccesses[i]->markBlockAndNeighborsNeedsUpdate(x, y, z);
    }
}

void World::func_48464_p(int_t x, int_t y, int_t z)
{
    if (lightingDirtyRegions.isActive())
    {
        lightingDirtyRegions.add(this, x, y, z);
        return;
    }
    markingFromLighting = true;
    for (IWorldAccess *access : worldAccesses)
        if (access != nullptr)
            access->markBlockAndNeighborsNeedsUpdate(x, y, z);
    markingFromLighting = false;
}

float World::getBrightness(int x, int y, int z, int minLight)
{
#if PLATFORM_FORCE_FULLBRIGHT_TERRAIN
    // Match ChunkCache::getBrightness() and World::getLightBrightness(): while PS2
    // skylight propagation is incomplete, every brightness accessor must report
    // full light or geometry colored through this path (block-edit re-renders via
    // the world-backed RenderBlocks, particles, entity shadows) comes out black.
    (void)x; (void)y; (void)z; (void)minLight;
    return 1.0f;
#endif
    int lightValue = getBlockLightValue(x, y, z);
    if (lightValue < minLight)
    {
        lightValue = minLight;
    }

    return worldProvider->lightBrightnessTable[lightValue];
}

float World::getLightBrightness(int x, int y, int z)
{
#if PLATFORM_FORCE_FULLBRIGHT_TERRAIN
    (void)x; (void)y; (void)z;
    return 1.0f;
#endif
    return worldProvider->lightBrightnessTable[getBlockLightValue(x, y, z)];
}

bool World::isDaytime()
{
    return skylightSubtracted < 4;
}

MovingObjectPosition* World::rayTraceBlocks(Vec3D* vec1, Vec3D* vec2)
{
    return rayTraceBlocks(vec1, vec2, false, false);
}

MovingObjectPosition* World::rayTraceBlocks_do(Vec3D* vec1, Vec3D* vec2, bool flag)
{
    return rayTraceBlocks(vec1, vec2, flag, false);
}

#if PLATFORM_FLOAT_VECTOR_MATH
namespace
{
// Single-precision form of the block ray march in World::rayTraceBlocks below.
// Step order, the plane selection and the byte0 face codes follow the double
// version line for line; two things differ.
//
// The walk is carried relative to the integer block the ray starts in, so single
// precision buys resolution along the ray instead of resolution against the
// world origin -- the latter is what makes plain float unusable for Minecraft
// coordinates. The ray is bounded on both ends: the block picker asks for about
// five blocks and the loop itself stops after 200 steps, so the largest offset
// this frame has to represent is small and its ulp stays far below the block
// grid. Plane targets are whole blocks, so they are computed as integers and
// converted once, which keeps every axis crossing exact.
//
// The per-iteration std::isnan probes are also gone. Each is an __unorddf2 call
// on the EE, and the R5900 FPU cannot hand one back: it has no NaN encoding and
// flushes instead. The only way d3/d4/d5 could have been NaN is a 0/0 division,
// which needs flag2 set while d6 is zero -- impossible, because both endpoints
// having the same coordinate makes them floor to the same block and clears the
// flag. The caller still range-checks vec1 and vec2 before the march starts.
//
// vec1 is advanced in place in absolute world coordinates at the end of every
// step, because Block::collisionRayTrace reads it in world space.
MovingObjectPosition *rayMarchBlocksFloat(World *world, Vec3D *vec1, Vec3D *vec2,
                                          int_t startX, int_t startY, int_t startZ,
                                          int_t endX, int_t endY, int_t endZ,
                                          bool stopOnLiquid, bool ignoreNonCollidable)
{
    const int_t baseX = startX;
    const int_t baseY = startY;
    const int_t baseZ = startZ;

    float posX = (float)(vec1->xCoord - (double)baseX);
    float posY = (float)(vec1->yCoord - (double)baseY);
    float posZ = (float)(vec1->zCoord - (double)baseZ);
    const float endPosX = (float)(vec2->xCoord - (double)baseX);
    const float endPosY = (float)(vec2->yCoord - (double)baseY);
    const float endPosZ = (float)(vec2->zCoord - (double)baseZ);

    for (int_t l1 = 200; l1-- >= 0;)
    {
        if (startX == endX && startY == endY && startZ == endZ)
        {
            return nullptr;
        }

        bool flag2 = true;
        bool flag3 = true;
        bool flag4 = true;
        float d = 999.0f;
        float d1 = 999.0f;
        float d2 = 999.0f;

        if (endX > startX)
        {
            d = (float)(startX + 1 - baseX);
        }
        else if (endX < startX)
        {
            d = (float)(startX - baseX);
        }
        else
        {
            flag2 = false;
        }

        if (endY > startY)
        {
            d1 = (float)(startY + 1 - baseY);
        }
        else if (endY < startY)
        {
            d1 = (float)(startY - baseY);
        }
        else
        {
            flag3 = false;
        }

        if (endZ > startZ)
        {
            d2 = (float)(startZ + 1 - baseZ);
        }
        else if (endZ < startZ)
        {
            d2 = (float)(startZ - baseZ);
        }
        else
        {
            flag4 = false;
        }

        float d3 = 999.0f;
        float d4 = 999.0f;
        float d5 = 999.0f;
        const float d6 = endPosX - posX;
        const float d7 = endPosY - posY;
        const float d8 = endPosZ - posZ;

        if (flag2)
        {
            d3 = (d - posX) / d6;
        }

        if (flag3)
        {
            d4 = (d1 - posY) / d7;
        }

        if (flag4)
        {
            d5 = (d2 - posZ) / d8;
        }

        unsigned char byte0 = 0;

        if (d3 < d4 && d3 < d5)
        {
            if (endX > startX)
            {
                byte0 = 4;
            }
            else
            {
                byte0 = 5;
            }

            posX = d;
            posY += d7 * d3;
            posZ += d8 * d3;
        }
        else if (d4 < d5)
        {
            if (endY > startY)
            {
                byte0 = 0;
            }
            else
            {
                byte0 = 1;
            }

            posX += d6 * d4;
            posY = d1;
            posZ += d8 * d4;
        }
        else
        {
            if (endZ > startZ)
            {
                byte0 = 2;
            }
            else
            {
                byte0 = 3;
            }

            posX += d6 * d5;
            posY += d7 * d5;
            posZ = d2;
        }

        vec1->xCoord = (double)baseX + (double)posX;
        vec1->yCoord = (double)baseY + (double)posY;
        vec1->zCoord = (double)baseZ + (double)posZ;

        // floor(base + local) == base + floor(local) for an integer base, so the
        // block the march lands on is the one the double path would have picked.
        Vec3D *vec3d2 = Vec3D::createVector(vec1->xCoord, vec1->yCoord, vec1->zCoord);
        startX = JavaArithmetic::intAdd(baseX, MathHelper::floor_float(posX));
        vec3d2->xCoord = (double)startX;

        if (byte0 == 5)
        {
            startX = JavaArithmetic::intSub(startX, 1);
            vec3d2->xCoord++;
        }

        startY = JavaArithmetic::intAdd(baseY, MathHelper::floor_float(posY));
        vec3d2->yCoord = (double)startY;

        if (byte0 == 1)
        {
            startY = JavaArithmetic::intSub(startY, 1);
            vec3d2->yCoord++;
        }

        startZ = JavaArithmetic::intAdd(baseZ, MathHelper::floor_float(posZ));
        vec3d2->zCoord = (double)startZ;

        if (byte0 == 3)
        {
            startZ = JavaArithmetic::intSub(startZ, 1);
            vec3d2->zCoord++;
        }

        int_t j2 = world->getBlockId(startX, startY, startZ);
        int_t k2 = world->getBlockMetadata(startX, startY, startZ);
        Block* block1 = Block::blocksList[j2];

        if ((!ignoreNonCollidable || block1 == nullptr || block1->getCollisionBoundingBoxFromPool(world, startX, startY, startZ) != nullptr)
            && j2 > 0 && block1->canCollideCheck(k2, stopOnLiquid))
        {
            MovingObjectPosition* movingObjectPosition1 = block1->collisionRayTrace(world, startX, startY, startZ, vec1, vec2);
            if (movingObjectPosition1 != nullptr)
            {
                return movingObjectPosition1;
            }
        }
    }

    return nullptr;
}
}
#endif

MovingObjectPosition* World::rayTraceBlocks(Vec3D* vec1, Vec3D* vec2, bool flag, bool flag1)
{
    if (std::isnan(vec1->xCoord) || std::isnan(vec1->yCoord) || std::isnan(vec1->zCoord))
    {
        return nullptr;
    }
    
    if (std::isnan(vec2->xCoord) || std::isnan(vec2->yCoord) || std::isnan(vec2->zCoord))
    {
        return nullptr;
    }
    
    int endX = MathHelper::floor_double(vec2->xCoord);
    int endY = MathHelper::floor_double(vec2->yCoord);
    int endZ = MathHelper::floor_double(vec2->zCoord);
    int startX = MathHelper::floor_double(vec1->xCoord);
    int startY = MathHelper::floor_double(vec1->yCoord);
    int startZ = MathHelper::floor_double(vec1->zCoord);
    int blockId = getBlockId(startX, startY, startZ);
    int metadata = getBlockMetadata(startX, startY, startZ);
    Block* block = Block::blocksList[blockId];
    
    if ((!flag1 || block == nullptr || block->getCollisionBoundingBoxFromPool(this, startX, startY, startZ) != nullptr) 
        && blockId > 0 && block->canCollideCheck(metadata, flag))
    {
        MovingObjectPosition* movingObjectPosition = block->collisionRayTrace(this, startX, startY, startZ, vec1, vec2);
        if (movingObjectPosition != nullptr)
        {
            return movingObjectPosition;
        }
    }
    
#if PLATFORM_FLOAT_VECTOR_MATH
    return rayMarchBlocksFloat(this, vec1, vec2, startX, startY, startZ, endX, endY, endZ, flag, flag1);
#else
    for (int l1 = 200; l1-- >= 0;)
    {
        if (std::isnan(vec1->xCoord) || std::isnan(vec1->yCoord) || std::isnan(vec1->zCoord))
        {
            return nullptr;
        }

        if (startX == endX && startY == endY && startZ == endZ)
        {
            return nullptr;
        }
        
        bool flag2 = true;
        bool flag3 = true;
        bool flag4 = true;
        double d = 999.0;
        double d1 = 999.0;
        double d2 = 999.0;
        
        if (endX > startX)
        {
            d = (double)startX + 1.0;
        }
        else if (endX < startX)
        {
            d = (double)startX + 0.0;
        }
        else
        {
            flag2 = false;
        }
        
        if (endY > startY)
        {
            d1 = (double)startY + 1.0;
        }
        else if (endY < startY)
        {
            d1 = (double)startY + 0.0;
        }
        else
        {
            flag3 = false;
        }
        
        if (endZ > startZ)
        {
            d2 = (double)startZ + 1.0;
        }
        else if (endZ < startZ)
        {
            d2 = (double)startZ + 0.0;
        }
        else
        {
            flag4 = false;
        }
        
        double d3 = 999.0;
        double d4 = 999.0;
        double d5 = 999.0;
        double d6 = vec2->xCoord - vec1->xCoord;
        double d7 = vec2->yCoord - vec1->yCoord;
        double d8 = vec2->zCoord - vec1->zCoord;
        
        if (flag2)
        {
            d3 = (d - vec1->xCoord) / d6;
        }
        
        if (flag3)
        {
            d4 = (d1 - vec1->yCoord) / d7;
        }
        
        if (flag4)
        {
            d5 = (d2 - vec1->zCoord) / d8;
        }
        
        unsigned char byte0 = 0;
        
        if (d3 < d4 && d3 < d5)
        {
            if (endX > startX)
            {
                byte0 = 4;
            }
            else
            {
                byte0 = 5;
            }
            
            vec1->xCoord = d;
            vec1->yCoord += d7 * d3;
            vec1->zCoord += d8 * d3;
        }
        else if (d4 < d5)
        {
            if (endY > startY)
            {
                byte0 = 0;
            }
            else
            {
                byte0 = 1;
            }
            
            vec1->xCoord += d6 * d4;
            vec1->yCoord = d1;
            vec1->zCoord += d8 * d4;
        }
        else
        {
            if (endZ > startZ)
            {
                byte0 = 2;
            }
            else
            {
                byte0 = 3;
            }
            
            vec1->xCoord += d6 * d5;
            vec1->yCoord += d7 * d5;
            vec1->zCoord = d2;
        }
        
        Vec3D* vec3d2 = Vec3D::createVector(vec1->xCoord, vec1->yCoord, vec1->zCoord);
        startX = (int)(vec3d2->xCoord = MathHelper::floor_double(vec1->xCoord));
        
        if (byte0 == 5)
        {
            startX = JavaArithmetic::intSub(startX, 1);
            vec3d2->xCoord++;
        }
        
        startY = (int)(vec3d2->yCoord = MathHelper::floor_double(vec1->yCoord));
        
        if (byte0 == 1)
        {
            startY = JavaArithmetic::intSub(startY, 1);
            vec3d2->yCoord++;
        }
        
        startZ = (int)(vec3d2->zCoord = MathHelper::floor_double(vec1->zCoord));
        
        if (byte0 == 3)
        {
            startZ = JavaArithmetic::intSub(startZ, 1);
            vec3d2->zCoord++;
        }
        
        int j2 = getBlockId(startX, startY, startZ);
        int k2 = getBlockMetadata(startX, startY, startZ);
        Block* block1 = Block::blocksList[j2];
        
        if ((!flag1 || block1 == nullptr || block1->getCollisionBoundingBoxFromPool(this, startX, startY, startZ) != nullptr) 
            && j2 > 0 && block1->canCollideCheck(k2, flag))
        {
            MovingObjectPosition* movingObjectPosition1 = block1->collisionRayTrace(this, startX, startY, startZ, vec1, vec2);
            if (movingObjectPosition1 != nullptr)
            {
                return movingObjectPosition1;
            }
        }
    }

    return nullptr;
#endif
}

MovingObjectPosition *World::rayTraceBlocks_do_do(Vec3D *start, Vec3D *end, bool stopOnLiquid, bool ignoreNonCollidable)
{
    return rayTraceBlocks(start, end, stopOnLiquid, ignoreNonCollidable);
}


void World::playSoundAtEntity(Entity* entity, const jstring& soundName, float volume, float pitch)
{
    for (size_t i = 0; i < worldAccesses.size(); i++)
    {
        worldAccesses[i]->playSound(soundName, entity->posX, entity->posY - (double)entity->yOffset, entity->posZ, volume, pitch);
    }
}

void World::playSoundEffect(double x, double y, double z, const jstring& soundName, float volume, float pitch)
{
    for (size_t i = 0; i < worldAccesses.size(); i++)
    {
        worldAccesses[i]->playSound(soundName, x, y, z, volume, pitch);
    }
}

void World::playRecord(const jstring& recordName, int x, int y, int z)
{
    for (size_t i = 0; i < worldAccesses.size(); i++)
    {
        worldAccesses[i]->playRecord(recordName, x, y, z);
    }
}

void World::spawnParticle(const jstring& particleName, double x, double y, double z, double velX, double velY, double velZ)
{
    for (size_t i = 0; i < worldAccesses.size(); i++)
    {
        worldAccesses[i]->spawnParticle(particleName, x, y, z, velX, velY, velZ);
    }
}

bool World::addWeatherEffect(Entity* entity)
{
    weatherEffects.push_back(entity);
    return true;
}

bool World::entityJoinedWorld(Entity* entity)
{
    int chunkX = MathHelper::floor_double(entity->posX / 16.0);
    int chunkZ = MathHelper::floor_double(entity->posZ / 16.0);
    bool flag = false;
    
    if (entity->isPlayer())
    {
        flag = true;
    }

    if (flag || chunkExists(chunkX, chunkZ))
    {
        // Java can harmlessly keep the same object in an ArrayList more than once:
        // removing one reference does not destroy the object while another reference
        // still exists.  In C++ loadedEntityList owns its entries, so a duplicate turns
        // into a dangling pointer as soon as updateEntities() deletes the first copy.
        // changeWorld() exercises this path by joining the transferred player and then
        // calling spawnPlayerWithLoadedChunks() for that same player.
        if (std::find(loadedEntityList.begin(), loadedEntityList.end(), entity) != loadedEntityList.end())
            return true;

        if (entity->isPlayer())
        {
            EntityPlayer* entityPlayer = static_cast<EntityPlayer*>(entity);
            playerEntities.push_back(entityPlayer);
            updateAllPlayersSleepingFlag();
        }
        
        getChunkFromChunkCoords(chunkX, chunkZ)->addEntity(entity);
        loadedEntityList.push_back(entity);
        trackLoadedEntityPointer(entity);
        entityCountsDirty = true;
        obtainEntitySkin(entity);
        return true;
    }
    
    return false;
}

void World::obtainEntitySkin(Entity* entity)
{
    for (size_t i = 0; i < worldAccesses.size(); i++)
    {
        worldAccesses[i]->obtainEntitySkin(entity);
    }
}

void World::releaseEntitySkin(Entity* entity)
{
    for (size_t i = 0; i < worldAccesses.size(); i++)
    {
        worldAccesses[i]->releaseEntitySkin(entity);
    }
}

void World::onEntityRemoved(Entity* entity)
{
    if (villageCollectionObj != nullptr)
        villageCollectionObj->onEntityRemoved(entity);

    // Java's entity references keep their targets alive until the holder drops
    // the reference. In C++ the removed entity is destroyed immediately after
    // this hook, so old-AI EntityCreature targets must be cleared before that
    // happens or the next AI tick can dereference freed memory. Compare pointer
    // values only; the target is still valid here and is not dereferenced.
    for (Entity *loaded : loadedEntityList)
    {
        if (loaded == nullptr || loaded == entity)
            continue;
        EntityCreature *creature = dynamic_cast<EntityCreature *>(loaded);
        if (creature != nullptr && creature->getTarget() == entity)
            creature->setTarget(nullptr);
    }
}

void World::setEntityDead(Entity* entity)
{
    if (entity->riddenByEntity != nullptr)
    {
        entity->riddenByEntity->mountEntity(nullptr);
    }
    
    if (entity->ridingEntity != nullptr)
    {
        entity->mountEntity(nullptr);
    }
    
    entity->setEntityDead();
    
    if (entity->isPlayer())
    {
        auto it = std::find(playerEntities.begin(), playerEntities.end(), static_cast<EntityPlayer*>(entity));
        if (it != playerEntities.end())
        {
            playerEntities.erase(it);
        }
        updateAllPlayersSleepingFlag();
    }
}


void World::queueEntityForDestruction(Entity *entity)
{
	if (entity != nullptr && entity->isDead &&
		std::find(unloadedEntityList.begin(), unloadedEntityList.end(), entity) == unloadedEntityList.end())
		unloadedEntityList.push_back(entity);
}

void World::detachEntityForWorldChange(Entity *entity)
{
    if (entity == nullptr)
        return;

    loadedEntityList.erase(std::remove(loadedEntityList.begin(), loadedEntityList.end(), entity), loadedEntityList.end());
    untrackLoadedEntityPointer(entity);
    entityCountsDirty = true;
    unloadedEntityList.erase(std::remove(unloadedEntityList.begin(), unloadedEntityList.end(), entity), unloadedEntityList.end());
    weatherEffects.erase(std::remove(weatherEffects.begin(), weatherEffects.end(), entity), weatherEffects.end());

    if (entity->isPlayer())
        playerEntities.erase(std::remove(playerEntities.begin(), playerEntities.end(), static_cast<EntityPlayer*>(entity)), playerEntities.end());

    if (entity->addedToChunk)
    {
        Chunk *chunk = getChunkIfExists(entity->chunkCoordX, entity->chunkCoordZ);
        if (chunk != nullptr)
            chunk->removeEntity(entity);
    }
    if (chunkProvider != nullptr)
        chunkProvider->removeEntityFromLoadedChunks(entity);
    entity->addedToChunk = false;
}

void World::addWorldAccess(IWorldAccess* worldAccess)
{
    worldAccesses.push_back(worldAccess);
}

void World::removeWorldAccess(IWorldAccess* worldAccess)
{
    auto it = std::find(worldAccesses.begin(), worldAccesses.end(), worldAccess);
    if (it != worldAccesses.end())
    {
        worldAccesses.erase(it);
    }
}

std::vector<AxisAlignedBB*> &World::getCollidingBoundingBoxes(Entity* entity, AxisAlignedBB* aabb)
{
    collidingBoundingBoxes.clear();
    
#if PLATFORM_FAST_BLOCK_COLLISIONS
    platformCollectBlockCollisions(this, aabb, collidingBoundingBoxes);
#else
    int minX = MathHelper::floor_double(aabb->minX);
    int maxX = MathHelper::floor_double(aabb->maxX + 1.0);
    int minY = MathHelper::floor_double(aabb->minY);
    int maxY = MathHelper::floor_double(aabb->maxY + 1.0);
    int minZ = MathHelper::floor_double(aabb->minZ);
    int maxZ = MathHelper::floor_double(aabb->maxZ + 1.0);
    
    for (int x = minX; x < maxX; x++)
    {
        for (int z = minZ; z < maxZ; z++)
        {
            if (!blockExists(x, 64, z))
            {
                continue;
            }
            
            for (int y = minY - 1; y < maxY; y++)
            {
                Block* block = Block::blocksList[getBlockId(x, y, z)];
                if (block != nullptr)
                {
                    block->getCollidingBoundingBoxes(this, x, y, z, aabb, collidingBoundingBoxes);
                }
            }
        }
    }
#endif
    
    double d = 0.25;
    const auto& list = getEntitiesWithinAABBExcludingEntity(entity, aabb->expand(d, d, d));
    
    for (size_t j2 = 0; j2 < list.size(); j2++)
    {
        AxisAlignedBB* aabb1 = list[j2]->getBoundingBox();
        if (aabb1 != nullptr && aabb1->intersectsWith(aabb))
        {
            collidingBoundingBoxes.push_back(aabb1);
        }
        
        aabb1 = entity->getCollisionBox(list[j2]);
        if (aabb1 != nullptr && aabb1->intersectsWith(aabb))
        {
            collidingBoundingBoxes.push_back(aabb1);
        }
    }
    
    return collidingBoundingBoxes;
}

#if PLATFORM_EARLY_COLLISION_EXIT
bool World::hasCollidingBoundingBoxes(Entity *entity, AxisAlignedBB *aabb)
{
    if (platformHasBlockCollision(this, aabb, collidingBoundingBoxes))
        return true;

    const double expansion = 0.25;
    const auto &entities = getEntitiesWithinAABBExcludingEntity(
        entity, aabb->expand(expansion, expansion, expansion));

    for (Entity *other : entities)
    {
        AxisAlignedBB *collision = other->getBoundingBox();
        if (collision != nullptr && collision->intersectsWith(aabb))
            return true;

        collision = entity->getCollisionBox(other);
        if (collision != nullptr && collision->intersectsWith(aabb))
            return true;
    }

    return false;
}
#endif

#if PLATFORM_FLOAT_COLLISION_SWEEP
void World::collectCollisionSweep(Entity *entity, AxisAlignedBB *aabb, PlatformCollisionSweep &sweep)
{
    platformCollectBlockCollisionSweep(this, aabb, sweep);

    const double d = 0.25;
    const auto &list = getEntitiesWithinAABBExcludingEntity(entity, aabb->expand(d, d, d));

    for (size_t j2 = 0; j2 < list.size(); j2++)
    {
        AxisAlignedBB *aabb1 = list[j2]->getBoundingBox();
        if (aabb1 != nullptr && aabb1->intersectsWith(aabb))
        {
            platformAppendCollisionSweepBox(sweep, aabb1);
        }

        aabb1 = entity->getCollisionBox(list[j2]);
        if (aabb1 != nullptr && aabb1->intersectsWith(aabb))
        {
            platformAppendCollisionSweepBox(sweep, aabb1);
        }
    }
}
#endif

int World::calculateSkylightSubtracted(float partialTicks)
{
    float celestialAngle = getCelestialAngle(partialTicks);
    float f2 = 1.0f - (MathHelper::cos(celestialAngle * 3.1415927f * 2.0f) * 2.0f + 0.5f);
    
    if (f2 < 0.0f)
    {
        f2 = 0.0f;
    }
    
    if (f2 > 1.0f)
    {
        f2 = 1.0f;
    }
    
    f2 = 1.0f - f2;
    f2 = (float)((double)f2 * (1.0 - (double)(getRainStrengthInterpolated(partialTicks) * 5.0f) / 16.0));
    f2 = (float)((double)f2 * (1.0 - (double)(getThunderStrengthInterpolated(partialTicks) * 5.0f) / 16.0));
    f2 = 1.0f - f2;
    
    return (int)(f2 * 11.0f);
}

Vec3D* World::getSkyColor(Entity* entity, float partialTicks)
{
    float celestialAngle = getCelestialAngle(partialTicks);
    float f2 = MathHelper::cos(celestialAngle * 3.1415927f * 2.0f) * 2.0f + 0.5f;
    
    if (f2 < 0.0f)
    {
        f2 = 0.0f;
    }
    
    if (f2 > 1.0f)
    {
        f2 = 1.0f;
    }
    
    int i = MathHelper::floor_double(entity->posX);
    int j = MathHelper::floor_double(entity->posZ);
    BiomeGenBase *biome = getBiomeGenForCoords(i, j);
    float f3 = biome != nullptr ? biome->getFloatTemperature() : BiomeGenBase::plains->getFloatTemperature();
    int k = (biome != nullptr ? biome : BiomeGenBase::plains)->getSkyColorByTemp(f3);
    
    float f4 = (float)(k >> 16 & 0xff) / 255.0f;
    float f5 = (float)(k >> 8 & 0xff) / 255.0f;
    float f6 = (float)(k & 0xff) / 255.0f;
    
    f4 *= f2;
    f5 *= f2;
    f6 *= f2;
    
    float f7 = getRainStrengthInterpolated(partialTicks);
    if (f7 > 0.0f)
    {
        float f8 = (f4 * 0.3f + f5 * 0.59f + f6 * 0.11f) * 0.6f;
        float f10 = 1.0f - f7 * 0.75f;
        f4 = f4 * f10 + f8 * (1.0f - f10);
        f5 = f5 * f10 + f8 * (1.0f - f10);
        f6 = f6 * f10 + f8 * (1.0f - f10);
    }
    
    float f9 = getThunderStrengthInterpolated(partialTicks);
    if (f9 > 0.0f)
    {
        float f11 = (f4 * 0.3f + f5 * 0.59f + f6 * 0.11f) * 0.2f;
        float f13 = 1.0f - f9 * 0.75f;
        f4 = f4 * f13 + f11 * (1.0f - f13);
        f5 = f5 * f13 + f11 * (1.0f - f13);
        f6 = f6 * f13 + f11 * (1.0f - f13);
    }
    
    if (field_27172_i > 0)
    {
        float f12 = (float)field_27172_i - partialTicks;
        if (f12 > 1.0f)
        {
            f12 = 1.0f;
        }
        f12 *= 0.45f;
        f4 = f4 * (1.0f - f12) + 0.8f * f12;
        f5 = f5 * (1.0f - f12) + 0.8f * f12;
        f6 = f6 * (1.0f - f12) + 1.0f * f12;
    }
    
    return Vec3D::createVector(f4, f5, f6);
}

float World::getCelestialAngle(float partialTicks)
{
    return worldProvider->calculateCelestialAngle(worldInfo->getWorldTime(), partialTicks);
}

float World::func_35464_b(float partialTicks)
{
    float value = 1.0f - (MathHelper::cos(getCelestialAngle(partialTicks) * 3.14159265358979323846f * 2.0f) * 2.0f + 0.2f);
    if (value < 0.0f) value = 0.0f;
    if (value > 1.0f) value = 1.0f;
    value = 1.0f - value;
    value *= 1.0f - getRainStrengthInterpolated(partialTicks) * 5.0f / 16.0f;
    value *= 1.0f - getThunderStrengthInterpolated(partialTicks) * 5.0f / 16.0f;
    return value * 0.8f + 0.2f;
}

Vec3D* World::getCloudFogColor(float partialTicks)
{
    float celestialAngle = getCelestialAngle(partialTicks);
    float f2 = MathHelper::cos(celestialAngle * 3.1415927f * 2.0f) * 2.0f + 0.5f;
    
    if (f2 < 0.0f)
    {
        f2 = 0.0f;
    }
    
    if (f2 > 1.0f)
    {
        f2 = 1.0f;
    }
    
    float f3 = (float)(field_1019_F >> 16 & 255LL) / 255.0f;
    float f4 = (float)(field_1019_F >> 8 & 255LL) / 255.0f;
    float f5 = (float)(field_1019_F & 255LL) / 255.0f;
    
    float f6 = getRainStrengthInterpolated(partialTicks);
    if (f6 > 0.0f)
    {
        float f7 = (f3 * 0.3f + f4 * 0.59f + f5 * 0.11f) * 0.6f;
        float f9 = 1.0f - f6 * 0.95f;
        f3 = f3 * f9 + f7 * (1.0f - f9);
        f4 = f4 * f9 + f7 * (1.0f - f9);
        f5 = f5 * f9 + f7 * (1.0f - f9);
    }
    
    f3 *= f2 * 0.9f + 0.1f;
    f4 *= f2 * 0.9f + 0.1f;
    f5 *= f2 * 0.85f + 0.15f;
    
    float f8 = getThunderStrengthInterpolated(partialTicks);
    if (f8 > 0.0f)
    {
        float f10 = (f3 * 0.3f + f4 * 0.59f + f5 * 0.11f) * 0.2f;
        float f11 = 1.0f - f8 * 0.95f;
        f3 = f3 * f11 + f10 * (1.0f - f11);
        f4 = f4 * f11 + f10 * (1.0f - f11);
        f5 = f5 * f11 + f10 * (1.0f - f11);
    }
    
    return Vec3D::createVector(f3, f4, f5);
}

int_t World::getMoonPhase(float partialTicks)
{
    return worldProvider != nullptr ? worldProvider->getMoonPhase(worldInfo->getWorldTime(), partialTicks) : 0;
}

float World::getCelestialAngleRadians(float partialTicks)
{
    return getCelestialAngle(partialTicks) * 3.14159265358979323846f * 2.0f;
}

Vec3D *World::drawClouds(float partialTicks)
{
    return getCloudFogColor(partialTicks);
}

Vec3D* World::getFogColor(float partialTicks)
{
    float celestialAngle = getCelestialAngle(partialTicks);
    return worldProvider->getFogColor(celestialAngle, partialTicks);
}

int World::findTopSolidBlock(int x, int z)
{
    Chunk* chunk = getChunkFromBlockCoords(x, z);
    int y = WorldHeight::MAX_Y;
    x &= 0xf;
    z &= 0xf;
    
    while (y > 0)
    {
        int blockId = chunk->getBlockID(x, y, z);
        Material* material = (blockId != 0) ? Block::blocksList[blockId]->blockMaterial : Material::air;
        
        if (!material->getIsSolid() && !material->getIsLiquid())
        {
            y--;
        }
        else
        {
            return y + 1;
        }
    }
    
    return -1;
}

float World::getStarBrightness(float partialTicks)
{
    float celestialAngle = getCelestialAngle(partialTicks);
    float f2 = 1.0f - (MathHelper::cos(celestialAngle * 3.1415927f * 2.0f) * 2.0f + 0.75f);
    
    if (f2 < 0.0f)
    {
        f2 = 0.0f;
    }
    
    if (f2 > 1.0f)
    {
        f2 = 1.0f;
    }
    
    return f2 * f2 * 0.5f;
}

void World::scheduleBlockUpdate(int x, int y, int z, int blockId, int delay)
{
    const int RANGE = 8;

    // Immediate updates do not need a heap allocation. The previous code
    // allocated NextTickListEntry and returned without deleting it.
    if (scheduledUpdatesAreImmediate)
    {
        if (checkChunksExist(JavaArithmetic::intSub(x, RANGE), JavaArithmetic::intSub(y, RANGE), JavaArithmetic::intSub(z, RANGE),
                             JavaArithmetic::intAdd(x, RANGE), JavaArithmetic::intAdd(y, RANGE), JavaArithmetic::intAdd(z, RANGE)))
        {
            int currentBlockId = getBlockId(x, y, z);
            if (currentBlockId == blockId && currentBlockId > 0)
            {
                Block::blocksList[currentBlockId]->updateTick(this, x, y, z, rand);
            }
        }
        return;
    }

    if (!checkChunksExist(JavaArithmetic::intSub(x, RANGE), JavaArithmetic::intSub(y, RANGE), JavaArithmetic::intSub(z, RANGE),
                          JavaArithmetic::intAdd(x, RANGE), JavaArithmetic::intAdd(y, RANGE), JavaArithmetic::intAdd(z, RANGE)))
    {
        return;
    }

    const long_t scheduledTime = blockId > 0
        ? JavaArithmetic::longAdd(static_cast<long_t>(delay), worldInfo->getWorldTime())
        : 0;

#if PLATFORM_PC_LEGACY && PC_LEGACY_TICK_SCHEDULER
    if (pcLegacyTickScheduler == nullptr)
        pcLegacyTickScheduler = new PcLegacyTickScheduler();
    pcLegacyTickScheduler->schedule(x, y, z, blockId, scheduledTime);
#else
    NextTickListEntry* entry = new NextTickListEntry(x, y, z, blockId);
    entry->setScheduledTime(scheduledTime);

    auto inserted = scheduledTickSet.insert(entry);
    if (inserted.second)
    {
        scheduledTickTreeSet.insert(entry);
        scheduledTickOrder.add(entry);
    }
    else
    {
        // Duplicate update: the HashSet equivalent kept the old entry. The new
        // candidate must be destroyed or it leaks on every duplicate schedule.
        delete entry;
    }
#endif
}

void World::scheduleBlockUpdateFromLoad(int_t x, int_t y, int_t z, int_t blockId, int_t delay)
{
    const long_t scheduledTime = blockId > 0
        ? JavaArithmetic::longAdd(static_cast<long_t>(delay), worldInfo->getWorldTime())
        : 0;

#if PLATFORM_PC_LEGACY && PC_LEGACY_TICK_SCHEDULER
    if (pcLegacyTickScheduler == nullptr)
        pcLegacyTickScheduler = new PcLegacyTickScheduler();
    pcLegacyTickScheduler->schedule(x, y, z, blockId, scheduledTime);
#else
    NextTickListEntry *entry = new NextTickListEntry(x, y, z, blockId);
    entry->setScheduledTime(scheduledTime);

    auto inserted = scheduledTickSet.insert(entry);
    if (inserted.second)
    {
        scheduledTickTreeSet.insert(entry);
        scheduledTickOrder.add(entry);
    }
    else
        delete entry;
#endif
}

std::vector<NextTickListEntry *> World::getPendingBlockUpdates(Chunk *chunk, bool remove)
{
    std::vector<NextTickListEntry *> result;
    if (chunk == nullptr)
        return result;

#if PLATFORM_PC_LEGACY && PC_LEGACY_TICK_SCHEDULER
    if (pcLegacyTickScheduler == nullptr)
        return result;
    return pcLegacyTickScheduler->getPendingForChunk(chunk->xPosition, chunk->zPosition, remove);
#else
    const int_t minX = JavaArithmetic::intShl(chunk->xPosition, 4);
    const int_t maxX = JavaArithmetic::intAdd(minX, 16);
    const int_t minZ = JavaArithmetic::intShl(chunk->zPosition, 4);
    const int_t maxZ = JavaArithmetic::intAdd(minZ, 16);

    const std::vector<NextTickListEntry *> pendingOrder = scheduledTickOrder.valuesInIterationOrder();
    for (NextTickListEntry *entry : pendingOrder)
    {
        if (entry == nullptr || entry->xCoord < minX || entry->xCoord >= maxX ||
            entry->zCoord < minZ || entry->zCoord >= maxZ)
            continue;

        result.push_back(entry);
        if (remove)
        {
            scheduledTickTreeSet.erase(entry);
            scheduledTickSet.erase(entry);
            scheduledTickOrder.remove(entry);
        }
    }

    return result;
#endif
}

#if PLATFORM_BOUNDED_PATHFIND
// Per-tick A* path search budget (round-robin across all mobs). Reset at the
// start of every updateEntities() pass and consumed by getPathToEntity()/
// getEntityPathToXYZ(). A single world ticks on PS2, so a file-static is enough.
static int_t s_pathfindBudgetThisTick = 0;
#endif

void World::trackLoadedEntityPointer(Entity *entity)
{
#if PLATFORM_PC_LEGACY
    if (entity != nullptr)
        loadedEntityPointerSet.insert(entity);
#elif PLATFORM_PS2
    if (entity != nullptr && !loadedEntityPointerIndexOverflow && !loadedEntityPointerIndex.insert(entity))
        loadedEntityPointerIndexOverflow = true;
#else
    (void)entity;
#endif
}

void World::untrackLoadedEntityPointer(Entity *entity)
{
#if PLATFORM_PC_LEGACY
    if (entity != nullptr)
        loadedEntityPointerSet.erase(entity);
#elif PLATFORM_PS2
    if (entity != nullptr && !loadedEntityPointerIndexOverflow)
        loadedEntityPointerIndex.erase(entity);
#else
    (void)entity;
#endif
}

void World::rebuildLoadedEntityPointerSet() const
{
#if PLATFORM_PC_LEGACY
    loadedEntityPointerSet.clear();
    loadedEntityPointerSet.insert(loadedEntityList.begin(), loadedEntityList.end());
#elif PLATFORM_PS2
    loadedEntityPointerIndex.clear();
    loadedEntityPointerIndexOverflow = false;
    for (Entity *entity : loadedEntityList)
    {
        if (entity != nullptr && !loadedEntityPointerIndex.insert(entity))
        {
            loadedEntityPointerIndexOverflow = true;
            break;
        }
    }
#endif
}

bool World::isLoadedEntityPointer(const Entity *entity) const
{
    if (entity == nullptr)
        return false;
#if PLATFORM_PC_LEGACY
    if (loadedEntityPointerSet.size() != loadedEntityList.size())
        rebuildLoadedEntityPointerSet();
    return loadedEntityPointerSet.find(entity) != loadedEntityPointerSet.end();
#elif PLATFORM_PS2
    if (loadedEntityPointerIndexOverflow)
    {
        if (loadedEntityList.size() <= Ps2EntityPointerIndex::kMaxEntries)
            rebuildLoadedEntityPointerSet();
        if (loadedEntityPointerIndexOverflow)
            return std::find(loadedEntityList.begin(), loadedEntityList.end(), entity) != loadedEntityList.end();
    }
    else if (loadedEntityPointerIndex.size() != loadedEntityList.size())
    {
        rebuildLoadedEntityPointerSet();
        if (loadedEntityPointerIndexOverflow)
            return std::find(loadedEntityList.begin(), loadedEntityList.end(), entity) != loadedEntityList.end();
    }
    return loadedEntityPointerIndex.contains(entity);
#else
    return std::find(loadedEntityList.begin(), loadedEntityList.end(), entity) != loadedEntityList.end();
#endif
}

bool World::isLoadedTileEntityPointer(const TileEntity *tileEntity) const
{
    if (tileEntity == nullptr)
        return false;
    if (std::find(loadedTileEntityList.begin(), loadedTileEntityList.end(), tileEntity) != loadedTileEntityList.end())
        return true;
    return std::find(tileEntitiesToAdd.begin(), tileEntitiesToAdd.end(), tileEntity) != tileEntitiesToAdd.end();
}

void World::destroyEntity(Entity *entity)
{
    if (entity == nullptr)
        return;

    if (chunkProvider != nullptr)
        chunkProvider->removeEntityFromLoadedChunks(entity);
    entity->addedToChunk = false;
    deleteWorldOwnedEntity(entity);
}

void World::updateEntities()
{
#if PLATFORM_BOUNDED_PATHFIND
    s_pathfindBudgetThisTick = PLATFORM_PATHFIND_BUDGET_PER_TICK;
#endif
#if PLATFORM_PROFILE_RENDER_PHASES
    // This function is the whole of the client tick phase named "entities",
    // which measured 27.8ms of a 93.3ms frame for 48 loaded entities on PS2.
    // Split it so the per-entity simulation is separable from the list
    // bookkeeping, the unload drain and the tile-entity pass around it.
    long_t platformPhaseStartNs = System::nanoTime();
#endif
    // Update weather effects
    for (size_t i = 0; i < weatherEffects.size(); i++)
    {
        Entity* entity = weatherEffects[i];
        entity->onUpdate();
        
        if (entity->isDead)
        {
            weatherEffects.erase(weatherEffects.begin() + i);
            onEntityRemoved(entity);
            destroyEntity(entity);
            i--;
        }
    }
    
#if PLATFORM_PROFILE_RENDER_PHASES
    platformProfileTickPhase("entWeather", System::nanoTime() - platformPhaseStartNs);
    platformPhaseStartNs = System::nanoTime();
#endif
    // Java: loadedEntityList.removeAll(unloadedEntityList). Keep the unload
    // list in insertion order because it is iterated immediately afterwards for
    // chunk removal, skin release and entity-removal callbacks.
    if (!unloadedEntityList.empty())
    {
        for (Entity *entity : unloadedEntityList)
            untrackLoadedEntityPointer(entity);
#if PLATFORM_PC_LEGACY
        loadedEntityList.erase(
            std::remove_if(loadedEntityList.begin(), loadedEntityList.end(),
                [this](Entity* entity)
                {
                    return loadedEntityPointerSet.find(entity) == loadedEntityPointerSet.end();
                }),
            loadedEntityList.end());
#else
        loadedEntityList.erase(
            std::remove_if(loadedEntityList.begin(), loadedEntityList.end(),
                [this](Entity* entity)
                {
                    return std::find(unloadedEntityList.begin(), unloadedEntityList.end(), entity) != unloadedEntityList.end();
                }),
            loadedEntityList.end());
#endif
        entityCountsDirty = true;
    }

    // Remove unloaded entities from their chunks
    for (size_t j = 0; j < unloadedEntityList.size(); j++)
    {
        Entity* entity = unloadedEntityList[j];
        int chunkX = entity->chunkCoordX;
        int chunkZ = entity->chunkCoordZ;
        
        if (entity->addedToChunk)
        {
            Chunk *chunk = getChunkIfExists(chunkX, chunkZ);
            if (chunk != nullptr)
                chunk->removeEntity(entity);
        }
    }
    
    for (size_t j = 0; j < unloadedEntityList.size(); j++)
    {
        releaseEntitySkin(unloadedEntityList[j]);
        onEntityRemoved(unloadedEntityList[j]);
        destroyEntity(unloadedEntityList[j]);
    }
    
    unloadedEntityList.clear();
#if PLATFORM_PROFILE_RENDER_PHASES
    platformProfileTickPhase("entUnload", System::nanoTime() - platformPhaseStartNs);
    platformPhaseStartNs = System::nanoTime();
#endif

    // Update loaded entities
    for (size_t l = 0; l < loadedEntityList.size(); l++)
    {
        Entity* entity = loadedEntityList[l];
        
        if (entity->ridingEntity != nullptr)
        {
            if (!entity->ridingEntity->isDead && entity->ridingEntity->riddenByEntity == entity)
            {
                continue;
            }
            
            entity->ridingEntity->riddenByEntity = nullptr;
            entity->ridingEntity = nullptr;
        }
        
        if (!entity->isDead)
        {
#if PLATFORM_PS2 && MC_LOG_LEVEL > 2
            const std::uint32_t entityTickStart = platformProfileRenderPhaseBegin();
#endif
            updateEntity(entity);
#if PLATFORM_PS2 && MC_LOG_LEVEL > 2
            platformProfileEntityTick(entityTickStart, entity);
#endif
        }
        
        if (entity->isDead)
        {
            int chunkX = entity->chunkCoordX;
            int chunkZ = entity->chunkCoordZ;
            
            if (entity->addedToChunk)
            {
                Chunk *chunk = getChunkIfExists(chunkX, chunkZ);
                if (chunk != nullptr)
                    chunk->removeEntity(entity);
            }
            
            loadedEntityList.erase(
                loadedEntityList.begin() + static_cast<std::vector<Entity *>::difference_type>(l));
            untrackLoadedEntityPointer(entity);
            entityCountsDirty = true;
            l--;
            releaseEntitySkin(entity);
            onEntityRemoved(entity);
            destroyEntity(entity);
        }
    }
    
#if PLATFORM_PROFILE_RENDER_PHASES
    platformProfileTickPhase("entTick", System::nanoTime() - platformPhaseStartNs);
    platformPhaseStartNs = System::nanoTime();
#endif

    // Update tile entities
    updatingTileEntities = true;
    
    for (auto it = loadedTileEntityList.begin(); it != loadedTileEntityList.end();)
    {
        TileEntity* tileEntity = *it;
        
        if (!tileEntity->isInvalid() && tileEntity->worldObj != nullptr &&
            blockExists(tileEntity->xCoord, tileEntity->yCoord, tileEntity->zCoord))
        {
            tileEntity->updateEntity();
        }
        
        if (tileEntity->isInvalid())
        {
            notifyTileEntityRenderersRemoved(tileEntity);
            closeContainersUsing(tileEntity);
            it = loadedTileEntityList.erase(it);
            const int_t chunkX = JavaArithmetic::intShr(tileEntity->xCoord, 4);
            const int_t chunkZ = JavaArithmetic::intShr(tileEntity->zCoord, 4);
            Chunk *chunk = getChunkIfExists(chunkX, chunkZ);
            if (chunk != nullptr)
                chunk->removeChunkBlockTileEntity(tileEntity->xCoord & 0xf, tileEntity->yCoord, tileEntity->zCoord & 0xf);
            // The pending queue below is walked after this loop and would
            // otherwise dereference this pointer -- and could even reinstall it
            // into the chunk map. Java's list held a live reference, ours holds
            // an address that is about to be invalid.
            tileEntitiesToAdd.erase(std::remove(tileEntitiesToAdd.begin(), tileEntitiesToAdd.end(), tileEntity), tileEntitiesToAdd.end());
            delete tileEntity;
        }
        else
        {
            ++it;
        }
    }
    
    updatingTileEntities = false;

    if (!tileEntitiesToRemove.empty())
    {
        for (TileEntity *tileEntity : tileEntitiesToRemove)
        {
            loadedTileEntityList.erase(
                std::remove(loadedTileEntityList.begin(), loadedTileEntityList.end(), tileEntity),
                loadedTileEntityList.end());
        }
        tileEntitiesToRemove.clear();
    }
    
    if (!tileEntitiesToAdd.empty())
    {
        for (auto it = tileEntitiesToAdd.begin(); it != tileEntitiesToAdd.end();)
        {
            TileEntity* tileEntity = *it;
            
            if (!tileEntity->isInvalid())
            {
                auto findIt = std::find(loadedTileEntityList.begin(), loadedTileEntityList.end(), tileEntity);
                if (findIt == loadedTileEntityList.end())
                {
                    loadedTileEntityList.push_back(tileEntity);
                }
                
                const int_t chunkX = JavaArithmetic::intShr(tileEntity->xCoord, 4);
                const int_t chunkZ = JavaArithmetic::intShr(tileEntity->zCoord, 4);
                Chunk *chunk = getChunkIfExists(chunkX, chunkZ);
                if (chunk != nullptr)
                    chunk->setChunkBlockTileEntity(tileEntity->xCoord & 0xf, tileEntity->yCoord, tileEntity->zCoord & 0xf, tileEntity);
                
                markBlockNeedsUpdate(tileEntity->xCoord, tileEntity->yCoord, tileEntity->zCoord);
            }
            
            it = tileEntitiesToAdd.erase(it);
        }
    }
#if PLATFORM_PROFILE_RENDER_PHASES
    platformProfileTickPhase("entTile", System::nanoTime() - platformPhaseStartNs);
#endif
}

void World::addLoadedTileEntities(const std::vector<TileEntity*>& collection)
{
    if (updatingTileEntities)
    {
        for (TileEntity *tileEntity : collection)
        {
            if (tileEntity != nullptr &&
                std::find(loadedTileEntityList.begin(), loadedTileEntityList.end(), tileEntity) == loadedTileEntityList.end())
            {
                pushUniqueTileEntity(tileEntitiesToAdd, tileEntity);
            }
        }
    }
    else
    {
        for (TileEntity *tileEntity : collection)
            pushUniqueTileEntity(loadedTileEntityList, tileEntity);
    }
}

void World::ensureEntityChunkRetention(Entity *entity)
{
#if PLATFORM_ENTITY_CHUNK_RETENTION
    if (entity == nullptr || entity->isDead || entity->getChunkRetentionRadius() < 0)
        return;

    const int_t chunkX = MathHelper::floor_double(entity->posX / 16.0);
    const int_t chunkZ = MathHelper::floor_double(entity->posZ / 16.0);
    if (!chunkExists(chunkX, chunkZ))
        getChunkFromChunkCoords(chunkX, chunkZ);
#else
    (void)entity;
#endif
}

void World::prefetchEntityChunkRetention(Entity *entity)
{
#if PLATFORM_ENTITY_CHUNK_RETENTION
    if (entity == nullptr || entity->isDead || entity->getChunkRetentionRadius() < 0)
        return;

    const double lookAheadTicks = 8.0;
    const int_t chunkX = MathHelper::floor_double((entity->posX + entity->motionX * lookAheadTicks) / 16.0);
    const int_t chunkZ = MathHelper::floor_double((entity->posZ + entity->motionZ * lookAheadTicks) / 16.0);
    const int_t currentChunkX = MathHelper::floor_double(entity->posX / 16.0);
    const int_t currentChunkZ = MathHelper::floor_double(entity->posZ / 16.0);
    if ((chunkX != currentChunkX || chunkZ != currentChunkZ) &&
        !chunkExists(chunkX, chunkZ) && isChunkInLoadRadius(chunkX, chunkZ))
    {
        getChunkFromChunkCoords(chunkX, chunkZ);
    }
#else
    (void)entity;
#endif
}

void World::updateEntity(Entity* entity)
{
    updateEntityWithOptionalForce(entity, true);
}

void World::updateEntityWithOptionalForce(Entity* entity, bool flag)
{
#if PLATFORM_ENTITY_CHUNK_RETENTION
    if (flag)
        ensureEntityChunkRetention(entity);
#endif
    int i = MathHelper::floor_double(entity->posX);
    int j = MathHelper::floor_double(entity->posZ);
#if PLATFORM_BOUNDED_PATHFIND
    // Vanilla waits for a 32-block radius before ticking an entity. Bounded
    // console caches use their platform-specific margin so entities near the
    // streaming edge do not remain frozen waiting for a full 5x5 chunk area.
    const int RANGE = PLATFORM_PLAYER_UPDATE_CHUNK_RANGE_BLOCKS;
#else
    const int RANGE = 32;
#endif
    
    if (flag)
    {
        bool chunksExist;
#if PLATFORM_CACHE_ENTITY_CHUNK_EXISTENCE
        const int_t minChunkX = JavaArithmetic::intShr(JavaArithmetic::intSub(i, RANGE), 4);
        const int_t minChunkZ = JavaArithmetic::intShr(JavaArithmetic::intSub(j, RANGE), 4);
        const int_t maxChunkX = JavaArithmetic::intShr(JavaArithmetic::intAdd(i, RANGE), 4);
        const int_t maxChunkZ = JavaArithmetic::intShr(JavaArithmetic::intAdd(j, RANGE), 4);
        const std::uint32_t topologyVersion = chunkProvider->getChunkTopologyVersion();
        if (!entity->getCachedChunkExistence(minChunkX, minChunkZ, maxChunkX, maxChunkZ, topologyVersion, chunksExist))
        {
            chunksExist = checkChunksExist(JavaArithmetic::intSub(i, RANGE), 0, JavaArithmetic::intSub(j, RANGE),
                                           JavaArithmetic::intAdd(i, RANGE), WorldHeight::HEIGHT, JavaArithmetic::intAdd(j, RANGE));
            entity->cacheChunkExistence(minChunkX, minChunkZ, maxChunkX, maxChunkZ, topologyVersion, chunksExist);
        }
#else
        chunksExist = checkChunksExist(JavaArithmetic::intSub(i, RANGE), 0, JavaArithmetic::intSub(j, RANGE),
                                       JavaArithmetic::intAdd(i, RANGE), WorldHeight::HEIGHT, JavaArithmetic::intAdd(j, RANGE));
#endif
        if (!chunksExist)
        {
            return;
        }
    }
    
    entity->lastTickPosX = entity->posX;
    entity->lastTickPosY = entity->posY;
    entity->lastTickPosZ = entity->posZ;
    entity->prevRotationYaw = entity->rotationYaw;
    entity->prevRotationPitch = entity->rotationPitch;
    
    if (flag && entity->addedToChunk)
    {
        if (entity->ridingEntity != nullptr)
        {
            entity->updateRidden();
        }
        else
        {
            entity->onUpdate();
        }
    }
    
    if (std::isnan(entity->posX) || std::isinf(entity->posX))
    {
        entity->posX = entity->lastTickPosX;
    }
    
    if (std::isnan(entity->posY) || std::isinf(entity->posY))
    {
        entity->posY = entity->lastTickPosY;
    }
    
    if (std::isnan(entity->posZ) || std::isinf(entity->posZ))
    {
        entity->posZ = entity->lastTickPosZ;
    }
    
    if (std::isnan(entity->rotationPitch) || std::isinf(entity->rotationPitch))
    {
        entity->rotationPitch = entity->prevRotationPitch;
    }
    
    if (std::isnan(entity->rotationYaw) || std::isinf(entity->rotationYaw))
    {
        entity->rotationYaw = entity->prevRotationYaw;
    }
    
    int k = MathHelper::floor_double(entity->posX / 16.0);
    int l = MathHelper::floor_double(entity->posY / 16.0);
    int i1 = MathHelper::floor_double(entity->posZ / 16.0);
#if PLATFORM_ENTITY_CHUNK_RETENTION
    if (flag && entity->getChunkRetentionRadius() >= 0)
    {
        ensureEntityChunkRetention(entity);
        prefetchEntityChunkRetention(entity);
    }
#endif
    
    if (!entity->addedToChunk || entity->chunkCoordX != k || entity->chunkCoordY != l || entity->chunkCoordZ != i1)
    {
        if (entity->addedToChunk)
        {
            Chunk *oldChunk = getChunkIfExists(entity->chunkCoordX, entity->chunkCoordZ);
            if (oldChunk != nullptr)
                oldChunk->removeEntityAtIndex(entity, entity->chunkCoordY);
        }

        Chunk *newChunk = getChunkIfExists(k, i1);
        if (newChunk != nullptr)
        {
            entity->addedToChunk = true;
            newChunk->addEntity(entity);
        }
        else
        {
            entity->addedToChunk = false;
        }
    }
    
    if (flag && entity->addedToChunk && entity->riddenByEntity != nullptr)
    {
        if (entity->riddenByEntity->isDead || entity->riddenByEntity->ridingEntity != entity)
        {
            entity->riddenByEntity->ridingEntity = nullptr;
            entity->riddenByEntity = nullptr;
        }
        else
        {
            updateEntity(entity->riddenByEntity);
        }
    }
}

bool World::checkIfAABBIsClear(AxisAlignedBB* aabb)
{
    const auto& list = getEntitiesWithinAABBExcludingEntity(nullptr, aabb);
    
    for (size_t i = 0; i < list.size(); i++)
    {
        Entity* entity = list[i];
        if (!entity->isDead && entity->preventEntitySpawning)
        {
            return false;
        }
    }
    
    return true;
}

bool World::getIsAnyLiquid(AxisAlignedBB* aabb)
{
    int minX = MathHelper::floor_double(aabb->minX);
    int maxX = MathHelper::floor_double(aabb->maxX + 1.0);
    int minY = MathHelper::floor_double(aabb->minY);
    int maxY = MathHelper::floor_double(aabb->maxY + 1.0);
    int minZ = MathHelper::floor_double(aabb->minZ);
    int maxZ = MathHelper::floor_double(aabb->maxZ + 1.0);
    
    if (aabb->minX < 0.0)
    {
        minX--;
    }
    if (aabb->minY < 0.0)
    {
        minY--;
    }
    if (aabb->minZ < 0.0)
    {
        minZ--;
    }
    
    for (int x = minX; x < maxX; x++)
    {
        for (int y = minY; y < maxY; y++)
        {
            for (int z = minZ; z < maxZ; z++)
            {
                Block* block = Block::blocksList[getBlockId(x, y, z)];
                if (block != nullptr && block->blockMaterial->getIsLiquid())
                {
                    return true;
                }
            }
        }
    }
    
    return false;
}

bool World::isBoundingBoxBurning(AxisAlignedBB* aabb)
{
    int minX = MathHelper::floor_double(aabb->minX);
    int maxX = MathHelper::floor_double(aabb->maxX + 1.0);
    int minY = MathHelper::floor_double(aabb->minY);
    int maxY = MathHelper::floor_double(aabb->maxY + 1.0);
    int minZ = MathHelper::floor_double(aabb->minZ);
    int maxZ = MathHelper::floor_double(aabb->maxZ + 1.0);
    
    if (checkChunksExist(minX, minY, minZ, maxX, maxY, maxZ))
    {
        for (int x = minX; x < maxX; x++)
        {
            for (int y = minY; y < maxY; y++)
            {
                for (int z = minZ; z < maxZ; z++)
                {
                    int blockId = getBlockId(x, y, z);
                    if (blockId == Block::fire->blockID || 
                        blockId == Block::lavaMoving->blockID || 
                        blockId == Block::lavaStill->blockID)
                    {
                        return true;
                    }
                }
            }
        }
    }
    
    return false;
}

bool World::handleMaterialAcceleration(AxisAlignedBB* aabb, Material* material, Entity* entity)
{
    int minX = MathHelper::floor_double(aabb->minX);
    int maxX = MathHelper::floor_double(aabb->maxX + 1.0);
    int minY = MathHelper::floor_double(aabb->minY);
    int maxY = MathHelper::floor_double(aabb->maxY + 1.0);
    int minZ = MathHelper::floor_double(aabb->minZ);
    int maxZ = MathHelper::floor_double(aabb->maxZ + 1.0);
    
    if (!checkChunksExist(minX, minY, minZ, maxX, maxY, maxZ))
    {
        return false;
    }
    
    bool flag = false;
    Vec3D* vec3d = Vec3D::createVector(0.0, 0.0, 0.0);
    
    for (int x = minX; x < maxX; x++)
    {
        for (int y = minY; y < maxY; y++)
        {
            for (int z = minZ; z < maxZ; z++)
            {
                Block* block = Block::blocksList[getBlockId(x, y, z)];
                if (block == nullptr || block->blockMaterial != material)
                {
                    continue;
                }
                
                // Compare the fluid surface where it is produced, in float.
                //
                // Both operands of the subtraction are already float, so the
                // value is identical either way; storing it in a double only
                // widened it (__extendsfdf2), widened the int bound
                // (__floatsidf) and ran the comparison in software (__gedf2).
                // Y is a block height, so both sides stay exactly representable
                // and the test resolves the same way it did in double. This runs
                // once per cell of the entity's bounding volume, per entity, per
                // tick: three EE soft-float calls per cell become none.
                const float fluidSurface = (float)(y + 1) - BlockFluid::getPercentAir(getBlockMetadata(x, y, z));
                if ((float)maxY >= fluidSurface)
                {
                    flag = true;
                    block->velocityToAddToEntity(this, x, y, z, entity, vec3d);
                }
            }
        }
    }
    
    if (vec3d->lengthVector() > 0.0)
    {
        vec3d = vec3d->normalize();
        double d = 0.014;
        entity->motionX += vec3d->xCoord * d;
        entity->motionY += vec3d->yCoord * d;
        entity->motionZ += vec3d->zCoord * d;
    }
    
    return flag;
}

bool World::isMaterialInBB(AxisAlignedBB* aabb, Material* material)
{
    int minX = MathHelper::floor_double(aabb->minX);
    int maxX = MathHelper::floor_double(aabb->maxX + 1.0);
    int minY = MathHelper::floor_double(aabb->minY);
    int maxY = MathHelper::floor_double(aabb->maxY + 1.0);
    int minZ = MathHelper::floor_double(aabb->minZ);
    int maxZ = MathHelper::floor_double(aabb->maxZ + 1.0);
    
    for (int x = minX; x < maxX; x++)
    {
        for (int y = minY; y < maxY; y++)
        {
            for (int z = minZ; z < maxZ; z++)
            {
                Block* block = Block::blocksList[getBlockId(x, y, z)];
                if (block != nullptr && block->blockMaterial == material)
                {
                    return true;
                }
            }
        }
    }
    
    return false;
}

bool World::isAABBInMaterial(AxisAlignedBB* aabb, Material* material)
{
    int minX = MathHelper::floor_double(aabb->minX);
    int maxX = MathHelper::floor_double(aabb->maxX + 1.0);
    int minY = MathHelper::floor_double(aabb->minY);
    int maxY = MathHelper::floor_double(aabb->maxY + 1.0);
    int minZ = MathHelper::floor_double(aabb->minZ);
    int maxZ = MathHelper::floor_double(aabb->maxZ + 1.0);
    
    for (int x = minX; x < maxX; x++)
    {
        for (int y = minY; y < maxY; y++)
        {
            for (int z = minZ; z < maxZ; z++)
            {
                Block* block = Block::blocksList[getBlockId(x, y, z)];
                if (block == nullptr || block->blockMaterial != material)
                {
                    continue;
                }
                
                int metadata = getBlockMetadata(x, y, z);
                double d = y + 1;
                if (metadata < 8)
                {
                    d = (double)(y + 1) - (double)metadata / 8.0;
                }
                
                if (d >= aabb->minY)
                {
                    return true;
                }
            }
        }
    }
    
    return false;
}

Explosion *World::createExplosion(Entity* entity, double x, double y, double z, float strength)
{
    return newExplosion(entity, x, y, z, strength, false);
}

Explosion *World::newExplosion(Entity* entity, double x, double y, double z, float strength, bool isFlaming)
{
    Explosion *explosion = new Explosion(this, entity, x, y, z, strength);
    explosion->isFlaming = isFlaming;
    explosion->doExplosionA();
    explosion->doExplosionB(true);
    return explosion;
}

float World::getBlockDensity(Vec3D* vec, AxisAlignedBB* aabb) // func_675_a
{
#if PLATFORM_FLOAT_EXPLOSION_MATH
    // Rebase the sampling box around the explosion origin before narrowing.
    // Entity AABBs are only a few blocks wide, so the hot interpolation stays
    // precise even when the world coordinates themselves are large.
    const float densityMinX = static_cast<float>(aabb->minX - vec->xCoord);
    const float densityMinY = static_cast<float>(aabb->minY - vec->yCoord);
    const float densityMinZ = static_cast<float>(aabb->minZ - vec->zCoord);
    const float densityWidth = static_cast<float>(aabb->maxX - aabb->minX);
    const float densityHeight = static_cast<float>(aabb->maxY - aabb->minY);
    const float densityDepth = static_cast<float>(aabb->maxZ - aabb->minZ);
    const float densityStepX = 1.0f / (densityWidth * 2.0f + 1.0f);
    const float densityStepY = 1.0f / (densityHeight * 2.0f + 1.0f);
    const float densityStepZ = 1.0f / (densityDepth * 2.0f + 1.0f);

    int visible = 0;
    int total = 0;

    for (float f = 0.0f; f <= 1.0f; f += densityStepX)
    {
        for (float f1 = 0.0f; f1 <= 1.0f; f1 += densityStepY)
        {
            for (float f2 = 0.0f; f2 <= 1.0f; f2 += densityStepZ)
            {
                const float sampleOffsetX = densityMinX + densityWidth * f;
                const float sampleOffsetY = densityMinY + densityHeight * f1;
                const float sampleOffsetZ = densityMinZ + densityDepth * f2;
                Vec3D *sample = Vec3D::createVector(
                    vec->xCoord + static_cast<double>(sampleOffsetX),
                    vec->yCoord + static_cast<double>(sampleOffsetY),
                    vec->zCoord + static_cast<double>(sampleOffsetZ));

                MovingObjectPosition* mopDensity = rayTraceBlocks(sample, vec);
                if (mopDensity == nullptr)
                    visible++;
                delete mopDensity;
                total++;
            }
        }
    }

    return static_cast<float>(visible) / static_cast<float>(total);
#else
    double d = 1.0 / ((aabb->maxX - aabb->minX) * 2.0 + 1.0);
    double d1 = 1.0 / ((aabb->maxY - aabb->minY) * 2.0 + 1.0);
    double d2 = 1.0 / ((aabb->maxZ - aabb->minZ) * 2.0 + 1.0);
    
    int visible = 0;
    int total = 0;
    
    for (float f = 0.0f; f <= 1.0f; f = (float)((double)f + d))
    {
        for (float f1 = 0.0f; f1 <= 1.0f; f1 = (float)((double)f1 + d1))
        {
            for (float f2 = 0.0f; f2 <= 1.0f; f2 = (float)((double)f2 + d2))
            {
                double d3 = aabb->minX + (aabb->maxX - aabb->minX) * (double)f;
                double d4 = aabb->minY + (aabb->maxY - aabb->minY) * (double)f1;
                double d5 = aabb->minZ + (aabb->maxZ - aabb->minZ) * (double)f2;
                
                MovingObjectPosition* mopDensity = rayTraceBlocks(Vec3D::createVector(d3, d4, d5), vec);
                if (mopDensity == nullptr)
                    visible++;
                delete mopDensity;
                total++;
            }
        }
    }
    
    return (float)visible / (float)total;
#endif
}

void World::onBlockHit(EntityPlayer* player, int x, int y, int z, int side)
{
    if (side == 0) y--;
    if (side == 1) y++;
    if (side == 2) z--;
    if (side == 3) z++;
    if (side == 4) x--;
    if (side == 5) x++;
    
    if (getBlockId(x, y, z) == Block::fire->blockID)
    {
        playAuxSFXAtEntity(player, 1004, x, y, z, 0);
        setBlockWithNotify(x, y, z, 0);
    }
}

Entity* World::findEntityByClass(const std::type_info& classType)
{
    return nullptr;
}

std::string World::getLoadedEntityStats()
{
    return "All: " + std::to_string(loadedEntityList.size());
}

std::string World::getChunkProviderStats()
{
    return chunkProvider->makeString();
}

int_t World::getLoadedChunkCount() const
{
    return chunkProvider != nullptr ? chunkProvider->getLoadedChunkCount() : -1;
}

jstring World::getDebugLoadedEntities() const
{
    return jstring("All: ") + jstring(std::to_string(loadedEntityList.size()));
}

jstring World::getProviderName() const
{
    return chunkProvider != nullptr ? chunkProvider->makeString() : jstring();
}

void World::addTileEntity(const std::vector<TileEntity *> &tileEntities)
{
    addLoadedTileEntities(tileEntities);
}

TileEntity* World::getBlockTileEntity(int x, int y, int z)
{
    if (x < -30000000 || z < -30000000 || x >= 30000000 || z >= 30000000 || y < 0 || y >= WorldHeight::HEIGHT)
        return nullptr;
    Chunk *chunk = getChunkIfExists(JavaArithmetic::intShr(x, 4), JavaArithmetic::intShr(z, 4));
    if (chunk != nullptr)
    {
        return chunk->getChunkBlockTileEntity(x & 0xf, y, z & 0xf);
    }
    
    return nullptr;
}

void World::setBlockTileEntity(int x, int y, int z, TileEntity* tileEntity)
{
    if (tileEntity == nullptr || x < -30000000 || z < -30000000 || x >= 30000000 || z >= 30000000 || y < 0 || y >= WorldHeight::HEIGHT)
        return;
    if (!tileEntity->isInvalid())
    {
        if (updatingTileEntities)
        {
            tileEntity->xCoord = x;
            tileEntity->yCoord = y;
            tileEntity->zCoord = z;
            pushUniqueTileEntity(tileEntitiesToAdd, tileEntity);
        }
        else
        {
            pushUniqueTileEntity(loadedTileEntityList, tileEntity);
            Chunk* chunk = getChunkFromChunkCoords(JavaArithmetic::intShr(x, 4), JavaArithmetic::intShr(z, 4));
            if (chunk != nullptr)
            {
                chunk->setChunkBlockTileEntity(x & 0xf, y, z & 0xf, tileEntity);
            }
        }
    }
}

TileEntity *World::getPendingTileEntity(int_t x, int_t y, int_t z)
{
    // While updateEntities() is running, setBlockTileEntity() only queues the
    // tile entity here -- the chunk map is not touched until the pass ends. Java
    // let Chunk::getChunkBlockTileEntity return null through that window and the
    // orphan it had just built was collected. In C++ that orphan leaks on every
    // call and, worse, the null makes TileEntityFurnace::canInteractWith fail,
    // which closes the player's furnace GUI mid-smelt. Chunk asks here instead.
    for (TileEntity *tileEntity : tileEntitiesToAdd)
    {
        if (tileEntity != nullptr && !tileEntity->isInvalid() &&
            tileEntity->xCoord == x && tileEntity->yCoord == y && tileEntity->zCoord == z)
            return tileEntity;
    }
    return nullptr;
}

void World::markTileEntityChunkModified(int_t x, int_t y, int_t z, TileEntity *tileentity)
{
    if (blockExists(x, y, z))
    {
        Chunk *chunk = getChunkFromBlockCoords(x, z);
        chunk->setChunkModified();
#if PLATFORM_SAVE_RUNTIME_CHUNK_EDITS_ON_UNLOAD
        if (!isPopulationFastPathChunk(chunk))
            chunk->markRuntimeSaveRequired();
#endif
    }

    // Vanilla also tells every IWorldAccess here, but its
    // RenderGlobal::doNothingWithTileEntity() is an empty method. Ours scrubs
    // the tile entity out of the render lists (it is the removal hook, see
    // notifyTileEntityRenderersRemoved), and this path fires on every
    // inventory change through TileEntity::onInventoryChanged(): calling it
    // here unregistered a chest from rendering each time it was used, until
    // something else happened to rebuild the chunk mesh.
    (void)tileentity;
}

void World::updateTileEntityChunkAndDoNothing(int_t x, int_t y, int_t z, TileEntity *tileEntity)
{
    markTileEntityChunkModified(x, y, z, tileEntity);
}

void World::removeBlockTileEntity(int x, int y, int z)
{
    if (x < -30000000 || z < -30000000 || x >= 30000000 || z >= 30000000 || y < 0 || y >= WorldHeight::HEIGHT)
        return;
    TileEntity* tileEntity = getBlockTileEntity(x, y, z);
    
    if (tileEntity != nullptr)
        notifyTileEntityRenderersRemoved(tileEntity);

    if (tileEntity != nullptr && updatingTileEntities)
    {
        tileEntity->invalidate();
    }
    else
    {
        if (tileEntity != nullptr)
        {
            // Close before freeing, not after: the container is what still holds
            // this pointer, and its next isUsableByPlayer() would run through a
            // freed vtable.
            closeContainersUsing(tileEntity);
            loadedTileEntityList.erase(std::remove(loadedTileEntityList.begin(), loadedTileEntityList.end(), tileEntity), loadedTileEntityList.end());
            tileEntitiesToAdd.erase(std::remove(tileEntitiesToAdd.begin(), tileEntitiesToAdd.end(), tileEntity), tileEntitiesToAdd.end());
        }

        Chunk* chunk = getChunkFromChunkCoords(JavaArithmetic::intShr(x, 4), JavaArithmetic::intShr(z, 4));
        if (chunk != nullptr)
        {
            chunk->removeChunkBlockTileEntity(x & 0xf, y, z & 0xf);
        }

        // Chunk only removes its non-owning index entry: some callers (notably
        // updateEntities) already own the final delete.  This direct World API is
        // the owning removal path, so release the allocation after every world,
        // renderer and chunk reference has been scrubbed.
        delete tileEntity;
    }
}

void World::markTileEntityForDespawn(TileEntity *tileEntity)
{
    pushUniqueTileEntity(tileEntitiesToRemove, tileEntity);
}

bool World::isBlockOpaqueCube(int x, int y, int z)
{
    Block* block = Block::blocksList[getBlockId(x, y, z)];
    if (block == nullptr)
    {
        return false;
    }
    
    return block->isOpaqueCube();
}

bool World::isBlockNormalCube(int x, int y, int z)
{
    Block* block = Block::blocksList[getBlockId(x, y, z)];
    if (block == nullptr)
    {
        return false;
    }
    
    return block->blockMaterial->getIsTranslucent() && block->renderAsNormalBlock();
}

bool World::isBlockNormalCubeDefault(int_t x, int_t y, int_t z, bool defaultValue)
{
    if (x < -30000000 || z < -30000000 || x >= 30000000 || z >= 30000000)
        return defaultValue;

    if (chunkLocalDecoration.isActive())
    {
        if (!chunkLocalDecoration.contains(x, z))
            return defaultValue;
        const int_t localId = chunkLocalDecoration.getBlockId(x, y, z);
        Block *localBlock = localId > 0 && localId < Block::BLOCK_REGISTRY_SIZE ? Block::blocksList[localId] : nullptr;
        return localBlock != nullptr && localBlock->blockMaterial->isOpaque() && localBlock->renderAsNormalBlock();
    }

    Chunk *chunk = chunkProvider != nullptr ? chunkProvider->provideChunk(JavaArithmetic::intShr(x, 4), JavaArithmetic::intShr(z, 4)) : nullptr;
    if (chunk == nullptr || chunk->isEmptyChunk())
        return defaultValue;

    const int_t blockId = getBlockId(x, y, z);
    Block *block = blockId >= 0 && blockId < Block::BLOCK_REGISTRY_SIZE ? Block::blocksList[blockId] : nullptr;
    return block != nullptr && block->blockMaterial->isOpaque() && block->renderAsNormalBlock();
}


void World::saveWorldIndirectly(IProgressUpdate* progressUpdate)
{
    saveWorld(true, progressUpdate);
    ThreadedFileIOBase::threadedIOInstance.waitForFinish();
}

bool World::updatingLighting()
{
    if (lightingUpdatesCounter >= 50)
        return false;

    lightingUpdatesCounter++;

    try
    {
        // Do a bounded amount of light propagation per rendered frame.  This
        // queue is fed heavily when chunks arrive; draining vanilla's 500 jobs
        // at once is a visible CPU stall on the low-console profiles even
        // though generation itself has already been throttled.
        int count = PLATFORM_LIGHTING_UPDATES_PER_FRAME;

        // A short queue is interactive work -- a placed torch, a dug block --
        // not chunk streaming, which arrives thousands of jobs at a time. Let
        // it finish this frame so the section is dirtied once, with its final
        // light, instead of once per frame of propagation.
        const bool interactiveBurst = PLATFORM_LIGHTING_INTERACTIVE_BURST > 0 &&
            (int)lightingToUpdate.size() <= PLATFORM_LIGHTING_INTERACTIVE_QUEUE_MAX;
        if (interactiveBurst && count < PLATFORM_LIGHTING_INTERACTIVE_BURST)
            count = PLATFORM_LIGHTING_INTERACTIVE_BURST;

        // Wall-clock ceiling on top of that count.  One job here is a flood fill
        // over a box, so the count alone bounds the number of jobs but not the
        // frame; see PLATFORM_LIGHTING_BUDGET_US.  Read once, and only when the
        // budget is enabled, so the profiles that leave it at 0 keep the
        // original loop.
        const uint64_t budgetStartUs = PLATFORM_LIGHTING_BUDGET_US > 0
            ? PlatformCompat::getMonotonicMicros()
            : 0;
        // Clamped to the shared streaming allowance of this frame; the
        // interactive burst below ignores it the same way it ignores the
        // per-call ceiling.
        const uint64_t budgetUs = PLATFORM_LIGHTING_BUDGET_US > 0
            ? (uint64_t)PlatformStreamingFrameBudget::clampUs((long_t)PLATFORM_LIGHTING_BUDGET_US)
            : 0;
        PlatformStreamingFrameBudgetScope frameBudgetScope;

        // One render mark per touched section for the whole drain, issued when
        // this scope ends on any of the exits below.
        struct DirtyBatchScope
        {
            World *world;
            explicit DirtyBatchScope(World *w) : world(w) { world->lightingDirtyRegions.begin(); }
            ~DirtyBatchScope()
            {
                world->markingFromLighting = true;
                world->lightingDirtyRegions.end(world);
                world->markingFromLighting = false;
            }
            DirtyBatchScope(const DirtyBatchScope &) = delete;
            DirtyBatchScope &operator=(const DirtyBatchScope &) = delete;
        } dirtyBatchScope(this);

        while (lightingToUpdate.size() > 0)
        {
            if (--count <= 0)
            {
                lightingUpdatesCounter--;
                return true;
            }

            MetadataChunkBlock metadataChunkBlock = lightingToUpdate.back();
            lightingToUpdate.pop_back();
            // Cleared before the job runs, so propagation inside it can queue
            // this cell again exactly as it could when the queue was scanned.
            if (isSingleCellLightingJob(metadataChunkBlock))
            {
                lightingQueuedCells.erase(LightingQueueCellSet::key(
                    metadataChunkBlock.skyBlock, metadataChunkBlock.minX,
                    metadataChunkBlock.minY, metadataChunkBlock.minZ));
            }
            metadataChunkBlock.updateLight(this);

            // Returning true is what the callers already understand as "there is
            // still lighting queued", so yielding on the clock needs no new
            // protocol: the per-frame caller comes back next frame and the
            // preload drain (Minecraft::preloadWorld) simply re-enters and
            // starts a fresh budget until the queue is empty.
            if (PLATFORM_LIGHTING_BUDGET_US > 0 && !interactiveBurst)
            {
                const uint64_t nowUs = PlatformCompat::getMonotonicMicros();
                if (nowUs > budgetStartUs &&
                    nowUs - budgetStartUs >= budgetUs)
                {
                    lightingUpdatesCounter--;
                    return true;
                }
            }
        }

        lightingUpdatesCounter--;
        return false;
    }
    catch (...)
    {
        lightingUpdatesCounter--;
        throw;
    }
}

int_t World::getPendingLightingUpdateCount() const
{
    return (int_t)lightingToUpdate.size();
}

void World::updateAllLightTypes(int_t x, int_t y, int_t z)
{
    if (worldProvider != nullptr && !worldProvider->hasNoSky)
        updateLightByType(EnumSkyBlock::Sky, x, y, z);
    updateLightByType(EnumSkyBlock::Block, x, y, z);
}

int_t World::computeSkyLightValue(int_t currentLight, int_t x, int_t y, int_t z, int_t blockId, int_t opacity)
{
    (void)currentLight;
    (void)blockId;
    int_t result = 0;
    if (canBlockSeeTheSky(x, y, z))
    {
        result = 15;
    }
    else
    {
        if (opacity == 0)
            opacity = 1;
        result = std::max(result, getSavedLightValue(EnumSkyBlock::Sky, x - 1, y, z) - opacity);
        result = std::max(result, getSavedLightValue(EnumSkyBlock::Sky, x + 1, y, z) - opacity);
        result = std::max(result, getSavedLightValue(EnumSkyBlock::Sky, x, y - 1, z) - opacity);
        result = std::max(result, getSavedLightValue(EnumSkyBlock::Sky, x, y + 1, z) - opacity);
        result = std::max(result, getSavedLightValue(EnumSkyBlock::Sky, x, y, z - 1) - opacity);
        result = std::max(result, getSavedLightValue(EnumSkyBlock::Sky, x, y, z + 1) - opacity);
    }
    return result;
}

int_t World::computeBlockLightValue(int_t currentLight, int_t x, int_t y, int_t z, int_t blockId, int_t opacity)
{
    (void)currentLight;
    int_t result = (blockId >= 0 && blockId < Block::BLOCK_REGISTRY_SIZE) ? Block::lightValue[blockId] : 0;
    result = std::max(result, getSavedLightValue(EnumSkyBlock::Block, x - 1, y, z) - opacity);
    result = std::max(result, getSavedLightValue(EnumSkyBlock::Block, x + 1, y, z) - opacity);
    result = std::max(result, getSavedLightValue(EnumSkyBlock::Block, x, y - 1, z) - opacity);
    result = std::max(result, getSavedLightValue(EnumSkyBlock::Block, x, y + 1, z) - opacity);
    result = std::max(result, getSavedLightValue(EnumSkyBlock::Block, x, y, z - 1) - opacity);
    result = std::max(result, getSavedLightValue(EnumSkyBlock::Block, x, y, z + 1) - opacity);
    return result;
}

void World::updateLightByType(EnumSkyBlock *type, int_t x, int_t y, int_t z)
{
    if (type == nullptr)
        return;

#if PLATFORM_CONSOLE_LOW
    // The low-memory console backend deliberately keeps OptiCraft's deferred and
    // mergeable lighting scheduler.  It preserves the result while avoiding the
    // synchronous 32K-entry flood fill on the gameplay thread.
    scheduleLightingUpdate(type, x, y, z, x, y, z);
    return;
#else
    if (!doChunksNearChunkExist(x, y, z, 17))
        return;

    int_t readIndex = 0;
    int_t writeIndex = 0;
    const int_t oldLight = getSavedLightValue(type, x, y, z);
    int_t blockId = getBlockId(x, y, z);
    int_t opacity = getBlockLightOpacity(x, y, z);
    if (opacity == 0)
        opacity = 1;

    const int_t newLight = type == EnumSkyBlock::Sky
        ? computeSkyLightValue(oldLight, x, y, z, blockId, opacity)
        : computeBlockLightValue(oldLight, x, y, z, blockId, opacity);

    if (newLight > oldLight)
    {
        lightUpdateBlockList[writeIndex++] = 133152;
    }
    else if (newLight < oldLight)
    {
        lightUpdateBlockList[writeIndex++] = 133152 + (oldLight << 18);
        while (readIndex < writeIndex)
        {
            const int_t packed = lightUpdateBlockList[readIndex++];
            const int_t px = (packed & 63) - 32 + x;
            const int_t py = ((packed >> 6) & 63) - 32 + y;
            const int_t pz = ((packed >> 12) & 63) - 32 + z;
            const int_t expected = (packed >> 18) & 15;
            const int_t actual = getSavedLightValue(type, px, py, pz);
            if (actual != expected)
                continue;

            setLightValue(type, px, py, pz, 0);
            if (expected <= 0)
                continue;

            const int_t dx = JavaArithmetic::intAbs(JavaArithmetic::intSub(px, x));
            const int_t dy = JavaArithmetic::intAbs(JavaArithmetic::intSub(py, y));
            const int_t dz = JavaArithmetic::intAbs(JavaArithmetic::intSub(pz, z));
            if (dx + dy + dz >= 17)
                continue;

            static const int_t offsets[6][3] = {
                {-1, 0, 0}, {1, 0, 0}, {0, -1, 0},
                {0, 1, 0}, {0, 0, -1}, {0, 0, 1}
            };
            for (const auto &offset : offsets)
            {
                const int_t nx = px + offset[0];
                const int_t ny = py + offset[1];
                const int_t nz = pz + offset[2];
                const int_t neighborLight = getSavedLightValue(type, nx, ny, nz);
                int_t neighborOpacity = getBlockLightOpacity(nx, ny, nz);
                if (neighborOpacity == 0)
                    neighborOpacity = 1;
                const int_t propagated = expected - neighborOpacity;
                if (neighborLight == propagated && writeIndex < static_cast<int_t>(lightUpdateBlockList.size()))
                {
                    lightUpdateBlockList[writeIndex++] = (nx - x + 32)
                        | ((ny - y + 32) << 6)
                        | ((nz - z + 32) << 12)
                        | (propagated << 18);
                }
            }
        }
        readIndex = 0;
    }

    while (readIndex < writeIndex)
    {
        const int_t packed = lightUpdateBlockList[readIndex++];
        const int_t px = (packed & 63) - 32 + x;
        const int_t py = ((packed >> 6) & 63) - 32 + y;
        const int_t pz = ((packed >> 12) & 63) - 32 + z;
        const int_t current = getSavedLightValue(type, px, py, pz);
        blockId = getBlockId(px, py, pz);
        opacity = getBlockLightOpacity(px, py, pz);
        if (opacity == 0)
            opacity = 1;

        const int_t target = type == EnumSkyBlock::Sky
            ? computeSkyLightValue(current, px, py, pz, blockId, opacity)
            : computeBlockLightValue(current, px, py, pz, blockId, opacity);
        if (target == current)
            continue;

        setLightValue(type, px, py, pz, target);
        if (target <= current)
            continue;

        const int_t dx = JavaArithmetic::intAbs(JavaArithmetic::intSub(px, x));
        const int_t dy = JavaArithmetic::intAbs(JavaArithmetic::intSub(py, y));
        const int_t dz = JavaArithmetic::intAbs(JavaArithmetic::intSub(pz, z));
        if (dx + dy + dz >= 17 || writeIndex >= static_cast<int_t>(lightUpdateBlockList.size()) - 6)
            continue;

        static const int_t offsets[6][3] = {
            {-1, 0, 0}, {1, 0, 0}, {0, -1, 0},
            {0, 1, 0}, {0, 0, -1}, {0, 0, 1}
        };
        for (const auto &offset : offsets)
        {
            const int_t nx = px + offset[0];
            const int_t ny = py + offset[1];
            const int_t nz = pz + offset[2];
            if (getSavedLightValue(type, nx, ny, nz) < target)
            {
                lightUpdateBlockList[writeIndex++] = (nx - x + 32)
                    | ((ny - y + 32) << 6)
                    | ((nz - z + 32) << 12);
            }
        }
    }
#endif
}

void World::scheduleLightingUpdate(EnumSkyBlock* enumSkyBlock, int minX, int minY, int minZ, int maxX, int maxY, int maxZ)
{
    scheduleLightingUpdate_do(enumSkyBlock, minX, minY, minZ, maxX, maxY, maxZ, true);
}

void World::scheduleLightingUpdate_do(EnumSkyBlock* enumSkyBlock, int minX, int minY, int minZ, int maxX, int maxY, int maxZ, bool flag)
{
#if PLATFORM_CONSOLE_LOW
#if PLATFORM_FORCE_FULLBRIGHT_TERRAIN
    // Terrain is rendered fullbright, so the recursive sky/block light flood-fill
    // is pure wasted EE work and is the main multi-second hitch when a new chunk
    // is generated. Skip queuing light updates entirely under this profile.
    (void)enumSkyBlock; (void)minX; (void)minY; (void)minZ;
    (void)maxX; (void)maxY; (void)maxZ; (void)flag;
    return;
#endif
#endif
    if (worldProvider->hasNoSky && enumSkyBlock == EnumSkyBlock::Sky)
    {
        return;
    }

    if (batchPopulationLightingUpdate(enumSkyBlock, minX, minY, minZ, maxX, maxY, maxZ))
        return;
    
    lightingUpdatesScheduled++;
    
    struct CounterGuard
    {
        int_t &counter;
        ~CounterGuard() { --counter; }
    } counterGuard{lightingUpdatesScheduled};

        if (lightingUpdatesScheduled == 50)
        {
            return;
        }
        
        int midX = (maxX + minX) / 2;
        int midZ = (maxZ + minZ) / 2;
        
        if (!blockExists(midX, 64, midZ))
        {
            return;
        }
        
        if (getChunkFromBlockCoords(midX, midZ)->isEmptyChunk())
        {
            return;
        }
        
        const int size = (int)lightingToUpdate.size();

        // A single cell is the common case (neighbour propagation) and is
        // answered by the cell set below instead of the tail scan; the scan
        // stays for box jobs, which the set cannot represent.
        const bool singleCell = minX == maxX && minY == maxY && minZ == maxZ;
        const ulong_t cellKey = singleCell
            ? LightingQueueCellSet::key(enumSkyBlock, minX, minY, minZ)
            : 0;

        if (flag && !singleCell)
        {
            int checkCount = PLATFORM_LIGHTING_MERGE_SCAN;
            // PS2 probes a much wider recent tail than vanilla. Recursive light
            // propagation tends to schedule overlapping neighbours close together,
            // so this catches the duplicates without an O(queue) scan per block.
            if (checkCount > size)
                checkCount = size;
            
            for (int i = 0; i < checkCount; i++)
            {
                MetadataChunkBlock& metadataChunkBlock = lightingToUpdate[lightingToUpdate.size() - i - 1];
                if (metadataChunkBlock.skyBlock == enumSkyBlock &&
                    metadataChunkBlock.tryMerge(minX, minY, minZ, maxX, maxY, maxZ))
                {
                    return;
                }
            }
        }
        
        if (size >= PLATFORM_LIGHTING_QUEUE_HARD_CAP)
        {
            // A saturated PS2 queue is already several seconds of propagation
            // work. Do not spend the last heap on more stale jobs; nearby changes
            // will be scheduled again as the backlog drains.
            return;
        }
        // Inserted only once every early return above is behind us, so the set
        // never claims a cell the queue does not hold.
        if (singleCell && !lightingQueuedCells.insert(cellKey))
            return;
        lightingToUpdate.emplace_back(enumSkyBlock, minX, minY, minZ, maxX, maxY, maxZ);

        const int MAX_UPDATES = 1000000;
        if (lightingToUpdate.size() > MAX_UPDATES)
        {
            // Logging here would be too noisy; clearing the queue matches the original abort behavior.
            lightingToUpdate.clear();
            lightingQueuedCells.clear();
        }
}

void World::calculateInitialSkylight()
{
    int i = calculateSkylightSubtracted(1.0f);
    if (i != skylightSubtracted)
    {
        skylightSubtracted = i;
    }
}

void World::setAllowedMobSpawns(bool hostile, bool peaceful)
{
    spawnHostileMobs = hostile;
    spawnPeacefulMobs = peaceful;
}

void World::setNaturalMobSpawningEnabled(bool enabled)
{
    naturalMobSpawningEnabled = enabled;
}

void World::setAllowedSpawnTypes(bool hostile, bool peaceful)
{
    setAllowedMobSpawns(hostile, peaceful);
}


void World::tick()
{
    if (worldInfo != nullptr && worldInfo->isHardcoreModeEnabled() && difficultySetting < 3)
        difficultySetting = 3;

    if (worldProvider != nullptr && worldProvider->worldChunkMgr != nullptr)
        worldProvider->worldChunkMgr->cleanupCache();

#if PLATFORM_PROFILE_RENDER_PHASES
    long_t platformPhaseStartNs = System::nanoTime();
#endif
    updateWeather();
#if PLATFORM_PROFILE_RENDER_PHASES
    platformProfileTickPhase("worldWeather", System::nanoTime() - platformPhaseStartNs);
#endif

    if (isAllPlayersFullyAsleep())
    {
        // Minecraft 1.2.5 no longer performs the Beta sleep-spawn pass here.
        // Sleeping advances directly to the next day and wakes every player.
        const long_t nextDay = JavaArithmetic::longAdd(worldInfo->getWorldTime(), 24000LL);
        worldInfo->setWorldTime(nextDay - nextDay % 24000LL);
        wakeUpAllPlayers();
    }

#if !PLATFORM_SKIP_MOB_SPAWNING
    if (naturalMobSpawningEnabled)
    {
#if PLATFORM_PROFILE_RENDER_PHASES
        platformPhaseStartNs = System::nanoTime();
#endif
#if PLATFORM_PROFILE_STREAMING && MC_LOG_LEVEL >= 2
        const long_t mobSpawnStartNs = System::nanoTime();
#endif
        // 1.2.5 attempts hostile spawns every tick but passive/animal spawning only
        // every 400 world ticks. The Beta path called peaceful spawning every tick.
        const bool spawnPeacefulThisTick = spawnPeacefulMobs && (worldInfo->getWorldTime() % 400LL == 0LL);
        SpawnerAnimals::performSpawning(this, spawnHostileMobs, spawnPeacefulThisTick);
#if PLATFORM_PROFILE_STREAMING && MC_LOG_LEVEL >= 2
        platformProfileMobSpawn(System::nanoTime() - mobSpawnStartNs);
#endif
#if PLATFORM_PROFILE_RENDER_PHASES
        platformProfileTickPhase("mobSpawn", System::nanoTime() - platformPhaseStartNs);
#endif
    }
#endif

    chunkProvider->unload100OldestChunks();

    int i = calculateSkylightSubtracted(1.0f);
    if (i != skylightSubtracted)
    {
        skylightSubtracted = i;
        for (size_t j = 0; j < worldAccesses.size(); j++)
            worldAccesses[j]->updateAllRenderers();
    }

    const long_t time = JavaArithmetic::longAdd(worldInfo->getWorldTime(), 1LL);
#if !PLATFORM_DISABLE_RUNTIME_AUTOSAVE
    // autosavePeriod is PLATFORM_AUTOSAVE_PERIOD_TICKS. Keep the platform I/O
    // policy while preserving the vanilla point in the world-tick sequence.
    if (time % (long)autosavePeriod == 0LL)
    {
#if PLATFORM_RUNTIME_AUTOSAVE_LEVEL_DATA
        saveWorld(false, nullptr);
#else
        chunkProvider->saveChunks(false, nullptr);
#endif
    }
#else
    (void)autosavePeriod;
#endif

    worldInfo->setWorldTime(time);
#if PLATFORM_PROFILE_RENDER_PHASES
    platformPhaseStartNs = System::nanoTime();
#endif
#if PLATFORM_PROFILE_STREAMING && MC_LOG_LEVEL >= 2
#if PLATFORM_PC_LEGACY && PC_LEGACY_TICK_SCHEDULER
    platformProfileTickQueue((long long)(pcLegacyTickScheduler != nullptr ? pcLegacyTickScheduler->size() : 0));
#else
    platformProfileTickQueue((long long)scheduledTickTreeSet.size());
#endif
    const long_t tickUpdatesStartNs = System::nanoTime();
#endif
    TickUpdates(false);
#if PLATFORM_PROFILE_STREAMING && MC_LOG_LEVEL >= 2
    platformProfileTickUpdates(System::nanoTime() - tickUpdatesStartNs);
#endif
#if PLATFORM_PROFILE_RENDER_PHASES
    platformProfileTickPhase("blockUpdates", System::nanoTime() - platformPhaseStartNs);
    platformPhaseStartNs = System::nanoTime();
#endif
    updateBlocksAndPlayCaveSounds();
#if PLATFORM_PROFILE_RENDER_PHASES
    platformProfileTickPhase("randomBlocks", System::nanoTime() - platformPhaseStartNs);
#endif

    // Vanilla 1.2.5 updates villages after scheduled/random block ticks.
    if (villageCollectionObj != nullptr && !multiplayerWorld)
        villageCollectionObj->tick();
    if (villageSiegeObj != nullptr && !multiplayerWorld)
        villageSiegeObj->tick();
}

void World::calculateInitialWeather()
{
    if (worldInfo->getRaining())
    {
        rainingStrength = 1.0f;
        if (worldInfo->getThundering())
            thunderingStrength = 1.0f;
    }
}

void World::initializeWeatherStrengths()
{
    calculateInitialWeather();
}

void World::updateWeather()
{
    if (worldProvider->hasNoSky)
    {
        return;
    }
    
    if (field_27172_i > 0)
    {
        --field_27172_i;
    }
    
    int thunderTimeValue = worldInfo->getThunderTime();
    if (thunderTimeValue <= 0)
    {
        if (worldInfo->getThundering())
        {
            worldInfo->setThunderTime(rand.nextInt(12000) + 3600);
        }
        else
        {
            worldInfo->setThunderTime(rand.nextInt(0x29040) + 12000);
        }
    }
    else
    {
        thunderTimeValue--;
        worldInfo->setThunderTime(thunderTimeValue);
        if (thunderTimeValue <= 0)
        {
            worldInfo->setThundering(!worldInfo->getThundering());
        }
    }
    
    int rainTime = worldInfo->getRainTime();
    if (rainTime <= 0)
    {
        if (worldInfo->getRaining())
        {
            worldInfo->setRainTime(rand.nextInt(12000) + 12000);
        }
        else
        {
            worldInfo->setRainTime(rand.nextInt(0x29040) + 12000);
        }
    }
    else
    {
        rainTime--;
        worldInfo->setRainTime(rainTime);
        if (rainTime <= 0)
        {
            worldInfo->setRaining(!worldInfo->getRaining());
        }
    }
    
    prevRainingStrength = rainingStrength;
    if (worldInfo->getRaining())
    {
        rainingStrength = (float)((double)rainingStrength + 0.01);
    }
    else
    {
        rainingStrength = (float)((double)rainingStrength - 0.01);
    }
    
    if (rainingStrength < 0.0f)
    {
        rainingStrength = 0.0f;
    }
    if (rainingStrength > 1.0f)
    {
        rainingStrength = 1.0f;
    }
    
    prevThunderingStrength = thunderingStrength;
    if (worldInfo->getThundering())
    {
        thunderingStrength = (float)((double)thunderingStrength + 0.01);
    }
    else
    {
        thunderingStrength = (float)((double)thunderingStrength - 0.01);
    }
    
    if (thunderingStrength < 0.0f)
    {
        thunderingStrength = 0.0f;
    }
    if (thunderingStrength > 1.0f)
    {
        thunderingStrength = 1.0f;
    }
}

void World::clearWeather()
{
    worldInfo->setRainTime(0);
    worldInfo->setRaining(false);
    worldInfo->setThunderTime(0);
    worldInfo->setThundering(false);
}

void World::stopPrecipitation()
{
    clearWeather();
}

void World::func_48461_r()
{
    const int_t range = PLATFORM_RANDOM_TICK_CHUNK_RADIUS;

#if PLATFORM_CACHE_RANDOM_TICK_CHUNKS
    bool rebuildChunkSelection = !positionsToUpdateCacheValid;
    std::size_t playerChunkIndex = 0;
    if (!rebuildChunkSelection)
    {
        for (EntityPlayer *player : playerEntities)
        {
            if (player == nullptr)
                continue;

            const int_t chunkX = MathHelper::floor_double(player->posX / 16.0);
            const int_t chunkZ = MathHelper::floor_double(player->posZ / 16.0);
            const std::uint64_t key = packChunkCoordKey(chunkX, chunkZ);
            if (playerChunkIndex >= positionsToUpdatePlayerChunkKeys.size() ||
                positionsToUpdatePlayerChunkKeys[playerChunkIndex] != key)
            {
                rebuildChunkSelection = true;
                break;
            }
            ++playerChunkIndex;
        }

        if (!rebuildChunkSelection && playerChunkIndex != positionsToUpdatePlayerChunkKeys.size())
            rebuildChunkSelection = true;
    }

    if (rebuildChunkSelection)
    {
        positionsToUpdate.clear();
        positionsToUpdatePlayerChunkKeys.clear();

        for (EntityPlayer *player : playerEntities)
        {
            if (player == nullptr)
                continue;

            const int_t chunkX = MathHelper::floor_double(player->posX / 16.0);
            const int_t chunkZ = MathHelper::floor_double(player->posZ / 16.0);
            positionsToUpdatePlayerChunkKeys.push_back(packChunkCoordKey(chunkX, chunkZ));
            for (int_t dx = -range; dx <= range; ++dx)
                for (int_t dz = -range; dz <= range; ++dz)
                    positionsToUpdate.add(ChunkCoordIntPair(JavaArithmetic::intAdd(chunkX, dx), JavaArithmetic::intAdd(chunkZ, dz)));
        }

        std::vector<ChunkCoordIntPair> rebuiltOrder = positionsToUpdate.valuesInIterationOrder();
        positionsToUpdateOrder.swap(rebuiltOrder);
        positionsToUpdateCacheValid = true;
    }
#else
    positionsToUpdate.clear();
    for (EntityPlayer *player : playerEntities)
    {
        if (player == nullptr)
            continue;
        const int_t chunkX = MathHelper::floor_double(player->posX / 16.0);
        const int_t chunkZ = MathHelper::floor_double(player->posZ / 16.0);
        for (int_t dx = -range; dx <= range; ++dx)
            for (int_t dz = -range; dz <= range; ++dz)
                positionsToUpdate.add(ChunkCoordIntPair(JavaArithmetic::intAdd(chunkX, dx), JavaArithmetic::intAdd(chunkZ, dz)));
    }
#endif

    if (soundCounter > 0)
        --soundCounter;

    // Vanilla 1.2.5 probes one random block near a random player each tick so
    // local lighting changes do not wait for an unrelated block update.
    if (!playerEntities.empty())
    {
        EntityPlayer *player = playerEntities[(std::size_t)rand.nextInt((int_t)playerEntities.size())];
        if (player != nullptr)
        {
            const int_t x = JavaArithmetic::intSub(JavaArithmetic::intAdd(MathHelper::floor_double(player->posX), rand.nextInt(11)), 5);
            const int_t y = JavaArithmetic::intSub(JavaArithmetic::intAdd(MathHelper::floor_double(player->posY), rand.nextInt(11)), 5);
            const int_t z = JavaArithmetic::intSub(JavaArithmetic::intAdd(MathHelper::floor_double(player->posZ), rand.nextInt(11)), 5);
            updateAllLightTypes(x, y, z);
        }
    }
}

void World::func_48458_a(int_t chunkOriginX, int_t chunkOriginZ, Chunk *chunk)
{
    if (chunk == nullptr)
        return;

    chunk->updateSkylight();

    if (soundCounter == 0)
    {
        updateLCG = JavaArithmetic::intFromBits(
            static_cast<uint_t>(updateLCG) * 3u + static_cast<uint_t>(UPDATE_LCG_INCREMENT));
        const int_t randomBits = JavaArithmetic::intShr(updateLCG, 2);
        int_t localX = randomBits & 15;
        int_t localZ = JavaArithmetic::intShr(randomBits, 8) & 15;
        const int_t localY = JavaArithmetic::intShr(randomBits, 16) & 127;
        const int_t blockId = chunk->getBlockID(localX, localY, localZ);
        localX = JavaArithmetic::intAdd(localX, chunkOriginX);
        localZ = JavaArithmetic::intAdd(localZ, chunkOriginZ);

        if (blockId == 0 &&
            getFullBlockLightValue(localX, localY, localZ) <= rand.nextInt(8) &&
            getSavedLightValue(EnumSkyBlock::Sky, localX, localY, localZ) <= 0)
        {
            EntityPlayer *player = getClosestPlayer((double)localX + 0.5, (double)localY + 0.5,
                                                    (double)localZ + 0.5, 8.0);
            if (player != nullptr &&
                player->getDistanceSq((double)localX + 0.5, (double)localY + 0.5,
                                      (double)localZ + 0.5) > 4.0)
            {
                playSoundEffect((double)localX + 0.5, (double)localY + 0.5, (double)localZ + 0.5,
                                "ambient.cave.cave", 0.7f, 0.8f + rand.nextFloat() * 0.2f);
                soundCounter = rand.nextInt(12000) + 6000;
            }
        }
    }

    chunk->enqueueRelightChecks();
}

void World::updateBlocksAndPlayCaveSounds()
{
    func_48461_r();

#if PLATFORM_CACHE_RANDOM_TICK_CHUNKS
    const size_t totalChunks = positionsToUpdateOrder.size();
#else
    static std::vector<std::uint64_t> s_tickOrder;
    s_tickOrder.clear();
    for (const ChunkCoordIntPair &pair : positionsToUpdate.valuesInIterationOrder())
        s_tickOrder.push_back(packChunkCoordKey(pair.chunkXPos, pair.chunkZPos));

    const size_t totalChunks = s_tickOrder.size();
#endif
    if (totalChunks == 0)
        return;

    // How many of them this tick. Full desktop visits all of them; low-CPU
    // profiles can take a rotating slice so the phase cost is spread out.
    size_t visitCount = totalChunks;
#if PLATFORM_RANDOM_TICK_CHUNKS_PER_TICK > 0
    if (totalChunks > (size_t)PLATFORM_RANDOM_TICK_CHUNKS_PER_TICK)
        visitCount = (size_t)PLATFORM_RANDOM_TICK_CHUNKS_PER_TICK;
#endif

    static size_t s_tickCursor = 0;
    if (s_tickCursor >= totalChunks)
        s_tickCursor = 0;

    // Visiting fewer chunks must not slow the world down.
    //
    // The round-robin exists to spread the PER-CHUNK overhead -- the cave-sound
    // light probe, the snow branch's precipitation/temperature checks, the
    // chunk lookup itself -- which is paid once per chunk visited. The random
    // BLOCK ticks are different: they drive grass spread, crop and sapling
    // growth, leaf decay and fluid flow, and cutting their count by 25/N would
    // slow all of that by the same factor.
    //
    // So scale the per-visit count by exactly the rotation factor. Total random
    // ticks issued per world tick is then identical to visiting every chunk
    // (25 chunks x 8 == 5 chunks x 40), only concentrated in the slice being
    // visited -- which is fine, because their positions are random within the
    // chunk either way. On PC visitCount == totalChunks, so the factor is 1 and
    // nothing changes.
    const size_t randomTickScale = (totalChunks + visitCount - 1) / visitCount;

#if PLATFORM_RANDOM_TICK_PROFILE_INTERVAL > 0
    // Per-branch attribution. slowTick can only ever name the whole phase, so a
    // 180ms "randomBlocks" tick gave no way to tell the cave-sound probe from
    // the snow/ice branch from the block updateTicks. Each accumulator is this
    // tick's total for one branch; the statics hold the worst tick in the
    // window. If System::nanoTime is dead on this hardware every figure reads
    // 0.0 -- that is a clock problem, not a "nothing costs anything" result.
    long long profSoundNs = 0;
    long long profSnowNs = 0;
    long long profTicksNs = 0;
    long long profMark = 0;
    static long long s_profSoundMaxNs = 0;
    static long long s_profSnowMaxNs = 0;
    static long long s_profTicksMaxNs = 0;
    static int s_profTicksSeen = 0;
#endif

    for (size_t visited = 0; visited < visitCount; visited++)
    {
        const size_t tickIndex = (s_tickCursor + visited) % totalChunks;
#if PLATFORM_CACHE_RANDOM_TICK_CHUNKS
        const ChunkCoordIntPair &pair = positionsToUpdateOrder[tickIndex];
        const int_t pairChunkX = pair.chunkXPos;
        const int_t pairChunkZ = pair.chunkZPos;
#else
        const std::uint64_t chunkKey = s_tickOrder[tickIndex];
        const int_t pairChunkX = unpackChunkCoordX(chunkKey);
        const int_t pairChunkZ = unpackChunkCoordZ(chunkKey);
#endif
        int chunkX = JavaArithmetic::intMul(pairChunkX, 16);
        int chunkZ = JavaArithmetic::intMul(pairChunkZ, 16);
#if PLATFORM_BOUNDED_WORLD
        // These random ticks (cave sounds, snow, lightning) are cosmetic. Calling
        // getChunkFromChunkCoords() on a missing chunk GENERATES it, so this scan
        // force-generates the whole area every tick -- the multi-second worldTick
        // stall when entering new terrain, and unbounded growth against a fixed
        // chunk budget. Only tick resident chunks and let the chunk mesher drive
        // generation (spread one section per frame).
        if (!chunkProvider->chunkExists(pairChunkX, pairChunkZ))
            continue;
#endif
        Chunk* chunk = getChunkFromChunkCoords(pairChunkX, pairChunkZ);
        func_48458_a(chunkX, chunkZ, chunk);


        if (rand.nextInt(0x186a0) == 0 && isRaining() && isThundering())
        {
            updateLCG = JavaArithmetic::intFromBits(static_cast<uint_t>(updateLCG) * 3u + static_cast<uint_t>(UPDATE_LCG_INCREMENT));
            int randValue = JavaArithmetic::intShr(updateLCG, 2);
            const int_t lightningX = JavaArithmetic::intAdd(chunkX, randValue & 0xf);
            const int_t lightningZ = JavaArithmetic::intAdd(chunkZ, JavaArithmetic::intShr(randValue, 8) & 0xf);
            int lightningY = getPrecipitationHeight(lightningX, lightningZ);

            if (canLightningStrikeAt(lightningX, lightningY, lightningZ))
            {
                addWeatherEffect(new EntityLightningBolt(this, lightningX, lightningY, lightningZ));
                field_27172_i = 2;
            }
        }
        
        if (rand.nextInt(16) == 0)
        {
#if PLATFORM_RANDOM_TICK_PROFILE_INTERVAL > 0
            profMark = System::nanoTime();
#endif
            updateLCG = JavaArithmetic::intFromBits(static_cast<uint_t>(updateLCG) * 3u + static_cast<uint_t>(UPDATE_LCG_INCREMENT));
            int randValue = JavaArithmetic::intShr(updateLCG, 2);
            int snowX = randValue & 0xf;
            int snowZ = JavaArithmetic::intShr(randValue, 8) & 0xf;
            const int_t worldX = JavaArithmetic::intAdd(snowX, chunkX);
            const int_t worldZ = JavaArithmetic::intAdd(snowZ, chunkZ);
            const int_t snowY = getPrecipitationHeight(worldX, worldZ);

            const int_t belowSnowY = JavaArithmetic::intSub(snowY, 1);
            if (isBlockHydratedIndirectly(worldX, belowSnowY, worldZ))
                setBlockWithNotify(worldX, belowSnowY, worldZ, Block::ice->blockID);

            if (isRaining() && canSnowAt(worldX, snowY, worldZ))
                setBlockWithNotify(worldX, snowY, worldZ, Block::snow->blockID);
#if PLATFORM_RANDOM_TICK_PROFILE_INTERVAL > 0
            profSnowNs += System::nanoTime() - profMark;
#endif
        }

#if PLATFORM_RANDOM_TICK_PROFILE_INTERVAL > 0
        profMark = System::nanoTime();
#endif
        ExtendedBlockStorage **storage = chunk->getBlockStorageArray();
        auto tickRandomBlock = [&](ExtendedBlockStorage *section)
        {
            updateLCG = JavaArithmetic::intFromBits(
                static_cast<uint_t>(updateLCG) * 3u + static_cast<uint_t>(UPDATE_LCG_INCREMENT));
            int_t randValue = JavaArithmetic::intShr(updateLCG, 2);
            int_t tickX = randValue & 0xf;
            int_t tickZ = JavaArithmetic::intShr(randValue, 8) & 0xf;
            int_t tickY = JavaArithmetic::intShr(randValue, 16) & 0xf;
            int_t tickBlockId = section->getExtBlockID(tickX, tickY, tickZ);

            if (tickBlockId > 0 && tickBlockId < Block::BLOCK_REGISTRY_SIZE)
            {
                Block *block = Block::blocksList[tickBlockId];
                if (block != nullptr && Block::tickOnLoad[tickBlockId])
                {
                    block->updateTick(this,
                        JavaArithmetic::intAdd(tickX, chunkX),
                        JavaArithmetic::intAdd(tickY, section->getYLocation()),
                        JavaArithmetic::intAdd(tickZ, chunkZ),
                        rand);
                }
            }
        };

#if PLATFORM_RANDOM_BLOCK_TICKS_PER_CHUNK > 0
        ExtendedBlockStorage *tickableSections[Chunk::SECTION_COUNT];
        int_t tickableSectionCount = 0;
        for (int_t sectionIndex = 0; sectionIndex < Chunk::SECTION_COUNT; ++sectionIndex)
        {
            ExtendedBlockStorage *section = storage[sectionIndex];
            if (section == nullptr || !section->getNeedsRandomTick())
                continue;
            tickableSections[tickableSectionCount++] = section;
        }

        if (tickableSectionCount > 0)
        {
            const int_t attempts = (int_t)(PLATFORM_RANDOM_BLOCK_TICKS_PER_CHUNK * randomTickScale);
            const int_t sectionStart = (int_t)((s_tickCursor + visited) % (size_t)tickableSectionCount);
            for (int_t attempt = 0; attempt < attempts; ++attempt)
            {
                const int_t sectionOffset = attempt % tickableSectionCount;
                tickRandomBlock(tickableSections[(sectionStart + sectionOffset) % tickableSectionCount]);
            }
        }
#else
        for (int_t sectionIndex = 0; sectionIndex < Chunk::SECTION_COUNT; ++sectionIndex)
        {
            ExtendedBlockStorage *section = storage[sectionIndex];
            if (section == nullptr || !section->getNeedsRandomTick())
                continue;

            // Minecraft 1.2.5 performs exactly three random block probes per
            // tickable 16x16x16 section. Console builds rotate the active chunk
            // set to spread fixed per-chunk work, so preserve the same long-term
            // tick rate by applying the existing visit rotation factor here.
            const int_t attempts = (int_t)(3 * randomTickScale);
            for (int_t attempt = 0; attempt < attempts; ++attempt)
            {
                tickRandomBlock(section);
            }
        }
#endif
#if PLATFORM_RANDOM_TICK_PROFILE_INTERVAL > 0
        profTicksNs += System::nanoTime() - profMark;
#endif
    }

    s_tickCursor = (s_tickCursor + visitCount) % totalChunks;

#if PLATFORM_RANDOM_TICK_PROFILE_INTERVAL > 0
    if (profSoundNs > s_profSoundMaxNs) s_profSoundMaxNs = profSoundNs;
    if (profSnowNs  > s_profSnowMaxNs)  s_profSnowMaxNs  = profSnowNs;
    if (profTicksNs > s_profTicksMaxNs) s_profTicksMaxNs = profTicksNs;

    if (++s_profTicksSeen >= PLATFORM_RANDOM_TICK_PROFILE_INTERVAL)
    {
        MC_LOG_DEBUG("world", "randomBlocks worst tick over %d: sound=%.1fms snow=%.1fms ticks=%.1fms"
               " | chunks=%d/%d\n",
               (int)PLATFORM_RANDOM_TICK_PROFILE_INTERVAL,
               (double)s_profSoundMaxNs / 1000000.0,
               (double)s_profSnowMaxNs  / 1000000.0,
               (double)s_profTicksMaxNs / 1000000.0,
               (int)visitCount, (int)totalChunks);
        s_profSoundMaxNs = s_profSnowMaxNs = s_profTicksMaxNs = 0;
        s_profTicksSeen = 0;
    }
#endif
}

bool World::TickUpdates(bool flag)
{
#if PLATFORM_PC_LEGACY && PC_LEGACY_TICK_SCHEDULER
    int size = pcLegacyTickScheduler != nullptr ? static_cast<int>(pcLegacyTickScheduler->size()) : 0;
#else
    int size = scheduledTickTreeSet.size();
    if (size != scheduledTickSet.size())
    {
        throw std::runtime_error("TickNextTick list out of synch");
    }
#endif

    if (size > 1000)
    {
        size = 1000;
    }

    for (int j = 0; j < size; j++)
    {
#if PLATFORM_PC_LEGACY && PC_LEGACY_TICK_SCHEDULER
        NextTickListEntry *entry = pcLegacyTickScheduler->popNext(worldInfo->getWorldTime(), flag);
        if (entry == nullptr)
            break;
#else
        auto it = scheduledTickTreeSet.begin();
        NextTickListEntry* entry = *it;

        if (!flag && entry->scheduledTime > worldInfo->getWorldTime())
        {
            break;
        }

        scheduledTickTreeSet.erase(it);
        scheduledTickSet.erase(entry);
        scheduledTickOrder.remove(entry);
#endif

        const int RANGE = 8;
        if (!checkChunksExist(JavaArithmetic::intSub(entry->xCoord, RANGE),
                              JavaArithmetic::intSub(entry->yCoord, RANGE),
                              JavaArithmetic::intSub(entry->zCoord, RANGE),
                              JavaArithmetic::intAdd(entry->xCoord, RANGE),
                              JavaArithmetic::intAdd(entry->yCoord, RANGE),
                              JavaArithmetic::intAdd(entry->zCoord, RANGE)))
        {
            delete entry;
            continue;
        }

        int blockId = getBlockId(entry->xCoord, entry->yCoord, entry->zCoord);
        if (blockId == entry->blockID && blockId > 0)
        {
            Block::blocksList[blockId]->updateTick(this, entry->xCoord, entry->yCoord, entry->zCoord, rand);
        }
        delete entry;
    }

#if PLATFORM_PC_LEGACY && PC_LEGACY_TICK_SCHEDULER
    return pcLegacyTickScheduler != nullptr && !pcLegacyTickScheduler->empty();
#else
    return scheduledTickTreeSet.size() != 0;
#endif
}

bool World::tickUpdates(bool flag)
{
    return TickUpdates(flag);
}

void World::tickBlocksAndAmbiance()
{
    updateBlocksAndPlayCaveSounds();
}

void World::dropOldChunks()
{
    if (chunkProvider == nullptr)
        return;
    while (chunkProvider->unload100OldestChunks())
    {
    }
}

void World::randomDisplayUpdates(int x, int y, int z)
{
#if PLATFORM_SKIP_WORLD_PARTICLES
    // The only thing randomDisplayTick produces is particles — torch flames,
    // lava drips, smoke, portal sparkles. With particles skipped this loop runs
    // 1000 iterations of 6 RNG calls plus a getBlockId chunk lookup and then
    // throws every result away: measured as the 347ms `slowTick=randomDisplay`
    // spike in the FRAME log, the single largest tick phase after populate.
    // The particles knob was gating the spawn, not the search.
    (void)x; (void)y; (void)z;
    return;
#else
    const int RANGE = 16;
#if PLATFORM_REUSE_RANDOM_DISPLAY_RNG
    Random &random = randomDisplayRandom;
#else
    Random random;
#endif

#if PLATFORM_CACHE_RANDOM_DISPLAY_CHUNKS
    // nextIntOffset(..., 16) stays within 15 blocks of the player, so all
    // 1000 probes fit inside at most a 3x3 chunk window. Keep the vanilla RNG
    // draws and particle density, but resolve each touched chunk only once.
    const int_t centerChunkX = JavaArithmetic::intShr(x, 4);
    const int_t centerChunkZ = JavaArithmetic::intShr(z, 4);
    Chunk *nearbyChunks[3][3] = {};
    bool nearbyChunkCached[3][3] = {};
#endif

    for (int l = 0; l < PLATFORM_RANDOM_DISPLAY_PROBES; l++)
    {
        int randX = rand.nextIntOffset(x, RANGE);
        int randY = rand.nextIntOffset(y, RANGE);
        int randZ = rand.nextIntOffset(z, RANGE);
#if PLATFORM_CACHE_RANDOM_DISPLAY_CHUNKS
        int blockId = 0;
        if (randX >= -30000000 && randZ >= -30000000 && randX < 30000000 && randZ < 30000000 &&
            randY >= 0 && randY < WorldHeight::HEIGHT)
        {
            const int_t chunkX = JavaArithmetic::intShr(randX, 4);
            const int_t chunkZ = JavaArithmetic::intShr(randZ, 4);
            const int_t cacheX = JavaArithmetic::intSub(chunkX, centerChunkX) + 1;
            const int_t cacheZ = JavaArithmetic::intSub(chunkZ, centerChunkZ) + 1;

            if (static_cast<uint_t>(cacheX) < 3u && static_cast<uint_t>(cacheZ) < 3u)
            {
                if (!nearbyChunkCached[cacheX][cacheZ])
                {
                    nearbyChunks[cacheX][cacheZ] = getChunkFromChunkCoords(chunkX, chunkZ);
                    nearbyChunkCached[cacheX][cacheZ] = true;
                }

                blockId = nearbyChunks[cacheX][cacheZ]->getBlockID(randX & 0xf, randY, randZ & 0xf);
            }
            else
            {
                blockId = getBlockId(randX, randY, randZ);
            }
        }
#else
        int blockId = getBlockId(randX, randY, randZ);
#endif

        if (blockId == 0 && rand.nextInt(8) > randY && worldProvider->getWorldHasNoSky())
        {
            const float_t particleX = rand.nextFloat();
            const float_t particleY = rand.nextFloat();
            const float_t particleZ = rand.nextFloat();
            spawnParticle("depthsuspend",
                static_cast<double>(static_cast<float_t>(randX) + particleX),
                static_cast<double>(static_cast<float_t>(randY) + particleY),
                static_cast<double>(static_cast<float_t>(randZ) + particleZ),
                0.0, 0.0, 0.0);
        }
        else if (blockId > 0)
        {
            Block::blocksList[blockId]->randomDisplayTick(this, randX, randY, randZ, random);
        }
    }
#endif
}

std::vector<Entity*> &World::getEntitiesWithinAABBExcludingEntity(Entity* entity, AxisAlignedBB* aabb)
{
    entitiesWithinAABBExcludingEntity.clear();
    
    int minChunkX = MathHelper::floor_double((aabb->minX - 2.0) / 16.0);
    int maxChunkX = MathHelper::floor_double((aabb->maxX + 2.0) / 16.0);
    int minChunkZ = MathHelper::floor_double((aabb->minZ - 2.0) / 16.0);
    int maxChunkZ = MathHelper::floor_double((aabb->maxZ + 2.0) / 16.0);
    
    for (int cx = minChunkX; cx <= maxChunkX; cx++)
    {
        for (int cz = minChunkZ; cz <= maxChunkZ; cz++)
        {
            Chunk *chunk = getChunkIfExists(cx, cz);
            if (chunk != nullptr)
                chunk->getEntitiesWithinAABBForEntity(entity, aabb, entitiesWithinAABBExcludingEntity);
        }
    }
    
    return entitiesWithinAABBExcludingEntity;
}

std::vector<Entity*> World::getEntitiesWithinAABB(const std::type_info& classType, AxisAlignedBB* aabb)
{
    std::vector<Entity *> result;

    int minChunkX = MathHelper::floor_double((aabb->minX - 2.0) / 16.0);
    int maxChunkX = MathHelper::floor_double((aabb->maxX + 2.0) / 16.0);
    int minChunkZ = MathHelper::floor_double((aabb->minZ - 2.0) / 16.0);
    int maxChunkZ = MathHelper::floor_double((aabb->maxZ + 2.0) / 16.0);

    for (int cx = minChunkX; cx <= maxChunkX; cx++)
    {
        for (int cz = minChunkZ; cz <= maxChunkZ; cz++)
        {
            Chunk *chunk = getChunkIfExists(cx, cz);
            if (chunk != nullptr)
                chunk->getEntitiesOfTypeWithinAAAB(classType, aabb, result);
        }
    }

    return result;
}

Entity *World::findNearestEntityWithinAABB(const std::type_info &classType, AxisAlignedBB *aabb, Entity *excludingEntity)
{
    if (aabb == nullptr || excludingEntity == nullptr)
        return nullptr;

    const std::vector<Entity *> &entities = getEntitiesWithinAABB(classType, aabb);
    Entity *nearest = nullptr;
    double nearestDistanceSq = std::numeric_limits<double>::max();
    for (Entity *candidate : entities)
    {
        if (candidate == nullptr || candidate == excludingEntity || candidate->isDead)
            continue;
        double distanceSq = excludingEntity->getDistanceSqToEntity(candidate);
        if (distanceSq <= nearestDistanceSq)
        {
            nearest = candidate;
            nearestDistanceSq = distanceSq;
        }
    }
    return nearest;
}


Entity *World::getEntityByID(int_t entityId)
{
	for (Entity *entity : loadedEntityList)
	{
		if (entity != nullptr && entity->entityId == entityId)
			return entity;
	}
	return nullptr;
}

std::vector<Entity*> &World::getLoadedEntityList()
{
    return loadedEntityList;
}

int_t World::countEntities(EnumCreatureTypeTag tag)
{
	// Size alone is not a valid cache key: one mob can disappear while an
	// animal is added in the same tick, leaving the vector size unchanged but
	// the creature categories different. Explicitly invalidate on membership
	// changes so the Wii keeps the cheap cached counts without stale spawn caps.
	if (entityCountsDirty)
	{
		countedMonsters = 0;
		countedCreatures = 0;
		countedWaterCreatures = 0;
		for (Entity *entity : loadedEntityList)
		{
			if (entity->isMob()) countedMonsters++;
			if (entity->isAnimal()) countedCreatures++;
			if (entity->isWaterMob()) countedWaterCreatures++;
		}
		entityCountsDirty = false;
	}

	switch (tag)
	{
	case EnumCreatureTypeTag::monster_tag:
		return countedMonsters;
	case EnumCreatureTypeTag::creature_tag:
		return countedCreatures;
	case EnumCreatureTypeTag::waterCreature_tag:
		return countedWaterCreatures;
	}
	return 0;
}

void World::addLoadedEntities(const std::vector<Entity*>& list)
{
    // Dedup (Java relied on GC; here a duplicate entry becomes a dangling pointer after
    // the entity is deleted+removed once by updateEntities -> crash in the render loop's
    // typeid(*entity)). Matches the existing guard in onEntityAdded around line 2985.
    for (size_t i = 0; i < list.size(); i++)
    {
        Entity* entity = list[i];
        if (std::find(loadedEntityList.begin(), loadedEntityList.end(), entity) == loadedEntityList.end())
        {
            loadedEntityList.push_back(entity);
            entityCountsDirty = true;
        }
        trackLoadedEntityPointer(entity);
        obtainEntitySkin(entity);
    }
}

void World::unloadEntities(const std::vector<Entity*>& list)
{
    for (Entity *entity : list)
    {
        // A stale bucket entry may predate the authoritative cleanup in
        // destroyEntity(). Compare pointer values only before queueing an unload.
        if (std::find(loadedEntityList.begin(), loadedEntityList.end(), entity) == loadedEntityList.end())
            continue;

        // A server chunk-unload only removes the local player from that chunk's
        // spatial index.  Minecraft still owns and uses the player as its camera;
        // queueing it with ordinary mobs would make updateEntities() destroy it.
        if (isActiveClientEntity(entity))
        {
            entity->addedToChunk = false;
            continue;
        }

#if PLATFORM_ENTITY_CHUNK_RETENTION
        if (!entity->isDead && entity->getChunkRetentionRadius() >= 0)
        {
            entity->addedToChunk = false;
            continue;
        }
#endif

        if (std::find(unloadedEntityList.begin(), unloadedEntityList.end(), entity) == unloadedEntityList.end())
            unloadedEntityList.push_back(entity);
    }
}

void World::unloadAllChunks()
{
    while (chunkProvider->unload100OldestChunks());
}


bool World::canMineBlock(EntityPlayer *, int_t, int_t, int_t)
{
    return true;
}

bool World::extinguishFire(EntityPlayer *player, int_t x, int_t y, int_t z, int_t side)
{
    if (side == 0) --y;
    if (side == 1) ++y;
    if (side == 2) --z;
    if (side == 3) ++z;
    if (side == 4) --x;
    if (side == 5) ++x;
    if (Block::fire != nullptr && getBlockId(x, y, z) == Block::fire->blockID)
    {
        playAuxSFXAtEntity(player, 1004, x, y, z, 0);
        setBlockWithNotify(x, y, z, 0);
        return true;
    }
    return false;
}

bool World::canBlockBePlacedAt(int blockId, int x, int y, int z, bool flag, int side)
{
    int existingBlockId = getBlockId(x, y, z);
    Block* existingBlock = Block::blocksList[existingBlockId];
    Block* newBlock = Block::blocksList[blockId];
    AxisAlignedBB* aabb = newBlock->getCollisionBoundingBoxFromPool(this, x, y, z);
    
    if (flag)
    {
        aabb = nullptr;
    }
    
    if (aabb != nullptr && !checkIfAABBIsClear(aabb))
    {
        return false;
    }
    
    if (existingBlock == Block::waterMoving || existingBlock == Block::waterStill || 
        existingBlock == Block::lavaMoving || existingBlock == Block::lavaStill || 
        existingBlock == Block::fire || existingBlock == Block::snow)
    {
        existingBlock = nullptr;
    }
    
    return blockId > 0 && existingBlock == nullptr && newBlock->canPlaceBlockOnSide(this, x, y, z, side);
}

bool World::func_48457_a(EntityPlayer *player, int_t x, int_t y, int_t z, int_t side)
{
    return extinguishFire(player, x, y, z, side);
}

Entity *World::func_4085_a(const std::type_info &)
{
    return nullptr;
}

Entity *World::getSkyBlockType(const std::type_info &classType)
{
    return func_4085_a(classType);
}

Pathfinder *World::getReusablePathfinder(IBlockAccess *blockAccess, bool openDoors, bool breakDoors,
                                         bool avoidWater, bool canSwim)
{
    if (pathfinderCache == nullptr)
        pathfinderCache = new Pathfinder(blockAccess, openDoors, breakDoors, avoidWater, canSwim);
    else
        pathfinderCache->reset(blockAccess, openDoors, breakDoors, avoidWater, canSwim);
    return pathfinderCache;
}

PathEntity* World::getPathToEntity(Entity* entity, Entity* target, float maxRange)
{
    return getPathToEntity(entity, target, maxRange, true, false, false, false);
}

PathEntity* World::getPathToEntity(Entity* entity, Entity* target, float maxRange, bool openDoors, bool breakDoors, bool avoidWater, bool canSwim)
{
#if PLATFORM_BOUNDED_PATHFIND
    // Round-robin: only a few mobs may run a full A* search per tick. Mobs that
    // miss their turn keep their previous path (or none) and retry next tick.
    if (s_pathfindBudgetThisTick <= 0)
        return nullptr;
    s_pathfindBudgetThisTick--;
#endif
    int startX = MathHelper::floor_double(entity->posX);
    int startY = MathHelper::floor_double(entity->posY + 1.0);
    int startZ = MathHelper::floor_double(entity->posZ);
    int range = (int)(maxRange + 16.0f);
    int minX = startX - range;
    int minY = startY - range;
    int minZ = startZ - range;
    int maxX = startX + range;
    int maxY = startY + range;
    int maxZ = startZ + range;
    
    ChunkCache chunkCache(this, minX, minY, minZ, maxX, maxY, maxZ);
#if PLATFORM_REUSE_PATHFINDER
    return getReusablePathfinder(&chunkCache, openDoors, breakDoors, avoidWater, canSwim)
        ->createEntityPathTo(entity, target, maxRange);
#else
    Pathfinder pathfinder(&chunkCache, openDoors, breakDoors, avoidWater, canSwim);
    return pathfinder.createEntityPathTo(entity, target, maxRange);
#endif
}

PathEntity *World::getPathEntityToEntity(Entity *entity, Entity *target, float maxRange,
                                         bool openDoors, bool breakDoors, bool avoidWater, bool canSwim)
{
    return getPathToEntity(entity, target, maxRange, openDoors, breakDoors, avoidWater, canSwim);
}

PathEntity* World::getEntityPathToXYZ(Entity* entity, int x, int y, int z, float maxRange)
{
    return getEntityPathToXYZ(entity, x, y, z, maxRange, true, false, false, false);
}

PathEntity* World::getEntityPathToXYZ(Entity* entity, int x, int y, int z, float maxRange, bool openDoors, bool breakDoors, bool avoidWater, bool canSwim)
{
#if PLATFORM_BOUNDED_PATHFIND
    if (s_pathfindBudgetThisTick <= 0)
        return nullptr;
    s_pathfindBudgetThisTick--;
#endif
    int startX = MathHelper::floor_double(entity->posX);
    int startY = MathHelper::floor_double(entity->posY);
    int startZ = MathHelper::floor_double(entity->posZ);
    int range = (int)(maxRange + 8.0f);
    int minX = startX - range;
    int minY = startY - range;
    int minZ = startZ - range;
    int maxX = startX + range;
    int maxY = startY + range;
    int maxZ = startZ + range;
    
    ChunkCache chunkCache(this, minX, minY, minZ, maxX, maxY, maxZ);
#if PLATFORM_REUSE_PATHFINDER
    return getReusablePathfinder(&chunkCache, openDoors, breakDoors, avoidWater, canSwim)
        ->createEntityPathTo(entity, x, y, z, maxRange);
#else
    Pathfinder pathfinder(&chunkCache, openDoors, breakDoors, avoidWater, canSwim);
    return pathfinder.createEntityPathTo(entity, x, y, z, maxRange);
#endif
}

bool World::isBlockProvidingPowerTo(int x, int y, int z, int side)
{
    int blockId = getBlockId(x, y, z);
    if (blockId == 0)
    {
        return false;
    }
    
    return Block::blocksList[blockId]->isIndirectlyPoweringTo(this, x, y, z, side);
}

bool World::isBlockGettingPowered(int x, int y, int z)
{
    if (isBlockProvidingPowerTo(x, y - 1, z, 0)) return true;
    if (isBlockProvidingPowerTo(x, y + 1, z, 1)) return true;
    if (isBlockProvidingPowerTo(x, y, z - 1, 2)) return true;
    if (isBlockProvidingPowerTo(x, y, z + 1, 3)) return true;
    if (isBlockProvidingPowerTo(x - 1, y, z, 4)) return true;
    return isBlockProvidingPowerTo(x + 1, y, z, 5);
}

bool World::isBlockIndirectlyProvidingPowerTo(int x, int y, int z, int side)
{
    if (isBlockNormalCube(x, y, z))
    {
        return isBlockGettingPowered(x, y, z);
    }
    
    int blockId = getBlockId(x, y, z);
    if (blockId == 0)
    {
        return false;
    }
    
    return Block::blocksList[blockId]->isPoweringTo(this, x, y, z, side);
}

bool World::isBlockIndirectlyGettingPowered(int x, int y, int z)
{
    if (isBlockIndirectlyProvidingPowerTo(x, y - 1, z, 0)) return true;
    if (isBlockIndirectlyProvidingPowerTo(x, y + 1, z, 1)) return true;
    if (isBlockIndirectlyProvidingPowerTo(x, y, z - 1, 2)) return true;
    if (isBlockIndirectlyProvidingPowerTo(x, y, z + 1, 3)) return true;
    if (isBlockIndirectlyProvidingPowerTo(x - 1, y, z, 4)) return true;
    return isBlockIndirectlyProvidingPowerTo(x + 1, y, z, 5);
}

EntityPlayer* World::getClosestPlayerToEntity(Entity* entity, double maxDistance)
{
    return getClosestPlayer(entity->posX, entity->posY, entity->posZ, maxDistance);
}

EntityPlayer* World::getClosestVulnerablePlayerToEntity(Entity *entity, double maxDistance)
{
    return entity != nullptr ? getClosestVulnerablePlayer(entity->posX, entity->posY, entity->posZ, maxDistance) : nullptr;
}

EntityPlayer *World::getClosestVulnerablePlayer(double x, double y, double z, double maxDistance)
{
    double closestDist = -1.0;
    EntityPlayer *closestPlayer = nullptr;
    for (EntityPlayer *player : playerEntities)
    {
        if (player == nullptr || player->capabilities.disableDamage)
            continue;

        const double distSq = player->getDistanceSq(x, y, z);
        if ((maxDistance < 0.0 || distSq < maxDistance * maxDistance) &&
            (closestDist < 0.0 || distSq < closestDist))
        {
            closestDist = distSq;
            closestPlayer = player;
        }
    }
    return closestPlayer;
}

EntityPlayer* World::getClosestPlayer(double x, double y, double z, double maxDistance)
{
#if PLATFORM_SINGLE_LOCAL_PLAYER
    // Single-player-only profiles can avoid the generic player vector scan. The common entity-AI path therefore has
    // exactly one local player, so avoid the generic vector scan and keep the
    // local distance arithmetic on the hardware float FPU.
    if (playerEntities.size() == 1)
    {
        EntityPlayer *player = playerEntities[0];
#if PLATFORM_FLOAT_ENTITY_AI_MATH
        const float dx = (float)(player->posX - x);
        const float dy = (float)(player->posY - y);
        const float dz = (float)(player->posZ - z);
        const float distSq = dx * dx + dy * dy + dz * dz;
        const float maxDistanceFloat = (float)maxDistance;
        if (maxDistance < 0.0 || distSq < maxDistanceFloat * maxDistanceFloat)
        {
            return player;
        }
#else
        const double distSq = player->getDistanceSq(x, y, z);
        if (maxDistance < 0.0 || distSq < maxDistance * maxDistance)
        {
            return player;
        }
#endif
        return nullptr;
    }
#endif

    double closestDist = -1.0;
    EntityPlayer* closestPlayer = nullptr;
    
    for (size_t i = 0; i < playerEntities.size(); i++)
    {
        EntityPlayer* player = playerEntities[i];
        double distSq = player->getDistanceSq(x, y, z);
        
        if ((maxDistance < 0.0 || distSq < maxDistance * maxDistance) && 
            (closestDist == -1.0 || distSq < closestDist))
        {
            closestDist = distSq;
            closestPlayer = player;
        }
    }
    
    return closestPlayer;
}

EntityPlayer *World::func_48456_a(double x, double z, double maxDistance)
{
    double closestDist = -1.0;
    EntityPlayer *closest = nullptr;
    for (EntityPlayer *player : playerEntities)
    {
        if (player == nullptr)
            continue;
        const double distSq = player->getDistanceSq(x, player->posY, z);
        if ((maxDistance < 0.0 || distSq < maxDistance * maxDistance) &&
            (closestDist < 0.0 || distSq < closestDist))
        {
            closestDist = distSq;
            closest = player;
        }
    }
    return closest;
}


EntityPlayer* World::getPlayerEntityByName(const jstring& name)
{
    for (size_t i = 0; i < playerEntities.size(); i++)
    {
        if (name == playerEntities[i]->username)
        {
            return playerEntities[i];
        }
    }
    
    return nullptr;
}

void World::setChunkData(int x, int y, int z, int width, int height, int depth, const std::vector<byte_t> &data)
{
    int chunkX = JavaArithmetic::intShr(x, 4);
    int chunkZ = JavaArithmetic::intShr(z, 4);
    int endChunkX = JavaArithmetic::intShr((x + width) - 1, 4);
    int endChunkZ = JavaArithmetic::intShr((z + depth) - 1, 4);
    int offset = 0;
    int minY = y;
    int maxY = y + height;
    
    if (minY < 0)
    {
        minY = 0;
    }
    if (maxY > WorldHeight::HEIGHT)
    {
        maxY = WorldHeight::HEIGHT;
    }
    
    for (int cx = chunkX; cx <= endChunkX; cx++)
    {
        int localMinX = x - cx * 16;
        int localMaxX = (x + width) - cx * 16;
        
        if (localMinX < 0)
        {
            localMinX = 0;
        }
        if (localMaxX > 16)
        {
            localMaxX = 16;
        }
        
        for (int cz = chunkZ; cz <= endChunkZ; cz++)
        {
            int localMinZ = z - cz * 16;
            int localMaxZ = (z + depth) - cz * 16;
            
            if (localMinZ < 0)
            {
                localMinZ = 0;
            }
            if (localMaxZ > 16)
            {
                localMaxZ = 16;
            }
            
            offset = getChunkFromChunkCoords(cx, cz)->setChunkData(const_cast<byte_t *>(data.data()), localMinX, minY, localMinZ, 
                                                                     localMaxX, maxY, localMaxZ, offset);
            markBlocksDirty(cx * 16 + localMinX, minY, cz * 16 + localMinZ,
                           cx * 16 + localMaxX, maxY, cz * 16 + localMaxZ);
        }
    }
}

void World::sendQuittingDisconnectingPacket()
{
}

void World::checkSessionLock()
{
    saveHandler->checkSessionLock();
}

void World::setWorldTime(long_t time)
{
    worldInfo->setWorldTime(time);
}

long_t World::getSeed() const
{
    return worldInfo != nullptr ? worldInfo->getSeed() : 0LL;
}

long_t World::getRandomSeed()
{
    return worldInfo->getRandomSeed();
}

Random &World::setRandomSeed(int_t x, int_t z, int_t salt)
{
    const ulong_t seedBits =
        static_cast<ulong_t>(static_cast<long_t>(x)) * UINT64_C(341873128712) +
        static_cast<ulong_t>(static_cast<long_t>(z)) * UINT64_C(132897987541) +
        static_cast<ulong_t>(getRandomSeed()) +
        static_cast<ulong_t>(static_cast<long_t>(salt));
    rand.setSeed(JavaArithmetic::longFromBits(seedBits));
    return rand;
}

long_t World::getWorldTime()
{
    return worldInfo->getWorldTime();
}

ChunkCoordinates World::getSpawnPoint()
{
    // Same Java copy semantics without forcing every C++ caller to own a heap object.
    return ChunkCoordinates(worldInfo->getSpawnX(), worldInfo->getSpawnY(), worldInfo->getSpawnZ());
}

void World::setSpawnPoint(ChunkCoordinates* coordinates)
{
    worldInfo->setSpawn(coordinates->x, coordinates->y, coordinates->z);
}

void World::joinEntityInSurroundings(Entity* entity)
{
    int chunkX = MathHelper::floor_double(entity->posX / 16.0);
    int chunkZ = MathHelper::floor_double(entity->posZ / 16.0);
    // Vanilla's 5x5. This used to be PLATFORM_CHUNK_CACHE_RADIUS under the
    // console profile, which happened to be 2 on the PS2 and so changed nothing;
    // on a platform with a larger cache it would have GROWN the swept area past
    // vanilla, which is the opposite of what a memory budget wants. The radius is
    // vanilla everywhere now and the guard below is what does the actual work.
    const int RANGE = 2;

    for (int x = chunkX - RANGE; x <= chunkX + RANGE; x++)
    {
        for (int z = chunkZ - RANGE; z <= chunkZ + RANGE; z++)
        {
#if PLATFORM_BOUNDED_WORLD
            // Do not force-generate the whole 5x5 area here (it stalls for seconds
            // in new terrain). Touch only resident chunks; the mesher and on-demand
            // physics generate what is actually needed, spread across frames.
            if (!chunkProvider->chunkExists(x, z))
                continue;
#endif
            getChunkFromChunkCoords(x, z);
        }
    }
    
    auto it = std::find(loadedEntityList.begin(), loadedEntityList.end(), entity);
    if (it == loadedEntityList.end())
    {
        loadedEntityList.push_back(entity);
        trackLoadedEntityPointer(entity);
        entityCountsDirty = true;
    }
}

bool World::loadTexture(EntityPlayer* player, int x, int y, int z)
{
    return true;
}

void World::setEntityState(Entity* entity, byte_t byte0) // func_9425_a
{
}

void World::updateEntityList()
{
    for (auto it = unloadedEntityList.begin(); it != unloadedEntityList.end();)
    {
        Entity* entity = *it;
        auto findIt = std::find(loadedEntityList.begin(), loadedEntityList.end(), entity);
        if (findIt != loadedEntityList.end())
        {
            loadedEntityList.erase(findIt);
            untrackLoadedEntityPointer(entity);
            entityCountsDirty = true;
        }
        ++it;
    }
    
    for (size_t i = 0; i < unloadedEntityList.size(); i++)
    {
        Entity* entity = unloadedEntityList[i];
        int chunkX = entity->chunkCoordX;
        int chunkZ = entity->chunkCoordZ;
        
        if (entity->addedToChunk)
        {
            Chunk *chunk = getChunkIfExists(chunkX, chunkZ);
            if (chunk != nullptr)
                chunk->removeEntity(entity);
        }
    }
    
    for (size_t j = 0; j < unloadedEntityList.size(); j++)
    {
        releaseEntitySkin(unloadedEntityList[j]);
    }
    
    unloadedEntityList.clear();
    
    for (size_t k = 0; k < loadedEntityList.size(); k++)
    {
        Entity* entity = loadedEntityList[k];
        
        if (entity->ridingEntity != nullptr)
        {
            if (!entity->ridingEntity->isDead && entity->ridingEntity->riddenByEntity == entity)
            {
                continue;
            }
            
            entity->ridingEntity->riddenByEntity = nullptr;
            entity->ridingEntity = nullptr;
        }
        
        if (entity->isDead)
        {
            int chunkX = entity->chunkCoordX;
            int chunkZ = entity->chunkCoordZ;
            
            if (entity->addedToChunk)
            {
                Chunk *chunk = getChunkIfExists(chunkX, chunkZ);
                if (chunk != nullptr)
                    chunk->removeEntity(entity);
            }
            
            loadedEntityList.erase(loadedEntityList.begin() + static_cast<std::ptrdiff_t>(k));
            untrackLoadedEntityPointer(entity);
            entityCountsDirty = true;
            k--;
            releaseEntitySkin(entity);
        }
    }
}

IChunkProvider* World::getIChunkProvider()
{
    return chunkProvider;
}

void World::playNoteAt(int x, int y, int z, int instrument, int pitch)
{
    int blockId = getBlockId(x, y, z);
    if (blockId > 0)
    {
        Block::blocksList[blockId]->powerBlock(this, x, y, z, instrument, pitch);
    }
}

ISaveHandler *World::getSaveHandler() const
{
    return saveHandler;
}

WorldInfo* World::getWorldInfo()
{
    return worldInfo;
}

void World::updateAllPlayersSleepingFlag()
{
    allPlayersSleeping = !playerEntities.empty();
    
    for (auto it = playerEntities.begin(); it != playerEntities.end(); ++it)
    {
        EntityPlayer* player = *it;
        if (!player->isPlayerSleeping())
        {
            allPlayersSleeping = false;
            break;
        }
    }
}

void World::wakeUpAllPlayers()
{
    allPlayersSleeping = false;
    
    for (auto it = playerEntities.begin(); it != playerEntities.end(); ++it)
    {
        EntityPlayer* player = *it;
        if (player->isPlayerSleeping())
        {
            player->wakeUpPlayer(false, false, true);
        }
    }
    
    stopPrecipitation();
}

bool World::isAllPlayersFullyAsleep()
{
    if (allPlayersSleeping && !multiplayerWorld)
    {
        for (auto it = playerEntities.begin(); it != playerEntities.end(); ++it)
        {
            EntityPlayer* player = *it;
            if (!player->isPlayerFullyAsleep())
            {
                return false;
            }
        }
        
        return true;
    }
    
    return false;
}

float World::getThunderStrengthInterpolated(float partialTicks)
{
    return (prevThunderingStrength + (thunderingStrength - prevThunderingStrength) * partialTicks) * getRainStrengthInterpolated(partialTicks);
}

float World::getRainStrengthInterpolated(float partialTicks)
{
    return prevRainingStrength + (rainingStrength - prevRainingStrength) * partialTicks;
}

float World::getRainStrength(float f)
{
    return getRainStrengthInterpolated(f);
}

float World::getThunderStrength(float f)
{
    return getThunderStrengthInterpolated(f);
}

float World::getWeightedThunderStrength(float partialTicks)
{
    return getThunderStrengthInterpolated(partialTicks);
}


void World::setRainStrength(float strength)
{
    prevRainingStrength = strength;
    rainingStrength = strength;
}

bool World::isThundering()
{
    return (double)getThunderStrengthInterpolated(1.0f) > 0.9;
}

bool World::isRaining()
{
    return (double)getRainStrengthInterpolated(1.0f) > 0.2;
}

bool World::canLightningStrikeAt(int x, int y, int z)
{
    if (!isRaining())
        return false;
    if (!canBlockSeeTheSky(x, y, z))
        return false;
    if (getPrecipitationHeight(x, z) > y)
        return false;

    BiomeGenBase *biome = getBiomeGenForCoords(x, z);
    return biome != nullptr && !biome->getEnableSnow() && biome->canSpawnLightningBolt();
}

bool World::canBlockBeRainedOn(int x, int y, int z)
{
    return canLightningStrikeAt(x, y, z);
}

bool World::isBlockHighHumidity(int_t x, int_t, int_t z)
{
    BiomeGenBase *biome = getBiomeGenForCoords(x, z);
    return biome != nullptr && biome->isHighHumidity();
}

double World::getSeaLevel() const
{
    return worldInfo != nullptr && worldInfo->getTerrainType() == WorldType::FLAT ? 0.0 : 63.0;
}


void World::setItemData(const jstring& key, MapDataBase* mapData)
{
    mapStorage->setData(key, mapData);
}

MapDataBase* World::loadItemData(const std::type_info& classType, const jstring& key)
{
    return mapStorage->loadData(classType, key);
}

MapDataBase* World::loadItemData(const jstring& key, MapDataFactory factory)
{
    return mapStorage->loadData(key, factory);
}

int World::getUniqueDataId(const jstring& key)
{
    return mapStorage->getUniqueDataId(key);
}

void World::playAuxSFX(int type, int x, int y, int z, int data)
{
    playAuxSFXAtEntity(nullptr, type, x, y, z, data);
}

void World::playAuxSFXAtEntity(EntityPlayer* player, int type, int x, int y, int z, int data)
{
    for (size_t i = 0; i < worldAccesses.size(); i++)
    {
        worldAccesses[i]->playAuxSFX(player, type, x, y, z, data);
    }
}
bool World::isSafeToSave(int_t counter)
{
    // Java func_650_a: this is not only a boolean check; it performs
    // incremental chunk saving while the pause menu shows "Saving level...".
    return saveAllChunks(counter);
}

void World::isTrapdoorOpen(int_t i, int_t j, int_t k, int_t l, int_t i1)
{
    playAuxSFXAtEntity(nullptr, i, j, k, l, i1);
}

void World::getTrapdoorFacing(EntityPlayer *entityplayer, int_t i, int_t j, int_t k, int_t l, int_t i1)
{
    playAuxSFXAtEntity(entityplayer, i, j, k, l, i1);
}


int_t World::getHeight()
{
    return WorldHeight::HEIGHT;
}

bool World::func_48452_a()
{
    return false;
}
