#pragma once

#include <vector>
#include <array>
#include <cstdint>
#include <set>
#include <unordered_set>
#include <typeinfo>

#include "platform/PlatformConfig.h"
#include "platform/PlatformTuning.h"
#include "platform/world/PopulationRegionAccessor.h"
#include "platform/world/LightingQueueCellSet.h"
#include "platform/world/LightingDirtyRegions.h"
#include "platform/world/ChunkLocalDecorationTarget.h"
#if PLATFORM_FLOAT_COLLISION_SWEEP
#include "platform/world/PlatformCollisionSweep.h"
#endif
#if PLATFORM_PS2
#include "ps2/world/Ps2EntityPointerIndex.h"
#endif
#include "java/Type.h"
#include "java/String.h"
#include "java/Random.h"
#include "java/HashSet.h"
#include "ChunkCoordIntPair.h"

#include "IBlockAccess.h"
#include "EnumSkyBlock.h"
#include "EnumCreatureType.h"
#include "NextTickListEntry.h"
#include "MetadataChunkBlock.h"  // stored by value in lightingToUpdate
#include "WorldHeight.h"
#include "MapDataBase.h"

class WorldProvider;
class WorldInfo;
class WorldSettings;
class MapStorage;
class ISaveHandler;
class IChunkProvider;
class IChunkLoader;
class Chunk;
class ChunkPosition;
class EntityPlayer;
class Entity;
class IWorldAccess;
class Vec3D;
class AxisAlignedBB;
class Material;
class MovingObjectPosition;
class TileEntity;
class Explosion;
class ChunkCoordinates;
class PathEntity;
class Pathfinder;
class BiomeGenBase;
class NBTTagCompound;
class IProgressUpdate;
class WorldChunkManager;
class VillageCollection;
class VillageSiege;
class SpawnListEntry;
#if PLATFORM_PC_LEGACY
class PcLegacyTickScheduler;
#endif

// net.minecraft.src.World
class World : public IBlockAccess
{
public:
	WorldChunkManager *getWorldChunkManager() override;
	SpawnListEntry *getRandomMob(const EnumCreatureType &type, int_t x, int_t y, int_t z);
	ChunkPosition *findClosestStructure(const jstring &name, int_t x, int_t y, int_t z);
	ChunkCoordinates *getEntrancePortalLocation();
	void func_6464_c();
	bool quickSaveWorld(int_t progressStage);
	int_t getBlockLightOpacity(int_t x, int_t y, int_t z);
	int_t func_48462_d(int_t x, int_t y, int_t z);
	void func_48464_p(int_t x, int_t y, int_t z);
	jstring getDebugLoadedEntities() const;
	jstring getProviderName() const;
	void markTileEntityForDespawn(TileEntity *tileEntity);
	bool isBlockNormalCubeDefault(int_t x, int_t y, int_t z, bool defaultValue);
	void setAllowedSpawnTypes(bool hostile, bool peaceful);
	PathEntity *getPathEntityToEntity(Entity *entity, Entity *target, float maxRange,
	                                  bool openDoors, bool breakDoors, bool avoidWater, bool canSwim);
	EntityPlayer *getClosestVulnerablePlayer(double x, double y, double z, double maxDistance);
	long_t getSeed() const;
	ISaveHandler *getSaveHandler() const;
	float getWeightedThunderStrength(float partialTicks);
	bool isBlockHighHumidity(int_t x, int_t y, int_t z);
	double getSeaLevel() const;

	World(ISaveHandler *isavehandler, const jstring &s, WorldProvider *worldprovider, long_t l);
	World(World *world, WorldProvider *worldprovider);
	World(ISaveHandler *isavehandler, const jstring &s, long_t l);
	World(ISaveHandler *isavehandler, const jstring &s, WorldSettings *settings);
	World(ISaveHandler *isavehandler, const jstring &s, WorldSettings *settings, WorldProvider *worldprovider);
	World(ISaveHandler *isavehandler, const jstring &s, long_t l, WorldProvider *worldprovider);
	~World() override;

protected:
	// Minecraft 1.2.5 names.  The OptiCraft names below remain as compatibility
	// hooks for subclasses that still override the older translated API.
	virtual IChunkProvider *createChunkProvider();
	virtual void generateSpawnPoint();
	virtual IChunkProvider *getChunkProvider();
	virtual void getInitialSpawnLocation();

public:
	virtual void setSpawnLocation();
	int_t getFirstUncoveredBlock(int_t i, int_t j);
	void emptyMethod1();
	void spawnPlayerWithLoadedChunks(EntityPlayer *entityplayer);
	void saveWorld(bool flag, IProgressUpdate *iprogressupdate);

private:
	void saveLevel();

public:
	bool getAnimationFrame(int_t i);
	bool saveAllChunks(int_t progressStage);
	int_t getBlockId(int_t i, int_t j, int_t k) override;
	bool isAirBlock(int_t i, int_t j, int_t k) override;
	bool blockExists(int_t i, int_t j, int_t k);
	bool doChunksNearChunkExist(int_t i, int_t j, int_t k, int_t l);
	bool checkChunksExist(int_t i, int_t j, int_t k, int_t l, int_t i1, int_t j1);
	bool chunkExists(int_t i, int_t j);
	Chunk *getChunkIfExists(int_t i, int_t j);
	// True if (cx,cz) is within the chunk provider's load radius and so is expected
	// to be generated even if it is not in memory yet (console deferred generation).
	bool isChunkInLoadRadius(int_t cx, int_t cz);
	bool isChunkPopulationPendingForRendering(int_t chunkX, int_t chunkZ) const;
	bool isChunkRetainedByEntity(int_t chunkX, int_t chunkZ) const;
	// True when the dimension keeps this chunk resident for its whole visit
	// (WorldProvider::getResidentChunkRadius); such chunks always exist for the
	// bounded chunk cache and are never evicted.
	bool isChunkResident(int_t chunkX, int_t chunkZ) const;
	bool isChunkRequiredByRetainedEntity(int_t chunkX, int_t chunkZ) const;

	Chunk *getChunkFromBlockCoords(int_t i, int_t j);
	Chunk *getChunkFromChunkCoords(int_t i, int_t j);
	virtual bool setBlockAndMetadata(int_t i, int_t j, int_t k, int_t l, int_t i1);
	virtual bool setBlock(int_t i, int_t j, int_t k, int_t l);
	Material *getBlockMaterial(int_t i, int_t j, int_t k) override;
	int_t getBlockMetadata(int_t i, int_t j, int_t k) override;
	void setBlockMetadataWithNotify(int_t i, int_t j, int_t k, int_t l);
	virtual bool setBlockMetadata(int_t i, int_t j, int_t k, int_t l);
	bool setBlockWithNotify(int_t i, int_t j, int_t k, int_t l);
	bool setBlockAndMetadataWithNotify(int_t i, int_t j, int_t k, int_t l, int_t i1);
	// Bounded-console population caches its already-loaded 2x2 footprint so the
	// ore/filler generators can avoid a provider lookup and the expensive light
	// and callback path for safe opaque-to-opaque replacements.
	void beginPopulationFastPath(int_t chunkX, int_t chunkZ);
	void endPopulationFastPath();
	// True while chunk is one of the tracked footprint chunks and a population
	// step is in progress, so Chunk::setBlockIDWithMetadata knows it is safe to
	// defer generateSkylightMap() to endPopulationFastPath() instead of
	// rescanning the whole column immediately -- see skylightRegenPending.
	bool isPopulationFastPathChunk(const Chunk *chunk) const;
	int_t getBlockIdForPopulation(int_t x, int_t y, int_t z);
	Chunk *getChunkForPopulation(int_t x, int_t y, int_t z);
	bool setBlockAndMetadataForPopulation(int_t x, int_t y, int_t z, int_t blockId, int_t metadata);
	bool replaceBlockForPopulation(int_t x, int_t y, int_t z,
	                               int_t expectedId, int_t newId);
	// Chunk-local decoration (PLATFORM_CHUNK_LOCAL_DECORATION): while bound,
	// block reads and writes are answered from the generator's buffer for the
	// chunk being generated and never reach the chunk provider. See
	// ChunkLocalDecorationTarget.
	void beginChunkLocalDecoration(int_t chunkX, int_t chunkZ, byte_t *blocks, byte_t *metadata,
	                               const std::vector<StructureBoundingBox> *structureBounds = nullptr);
	void endChunkLocalDecoration();
	bool isChunkLocalDecorationActive() const { return chunkLocalDecoration.isActive(); }
	void setChunkLocalDecorationStructureAvoidance(bool enabled)
	{
		chunkLocalDecoration.setStructureAvoidanceEnabled(enabled);
	}
	// True while a render-dirty mark is being issued because a light value
	// changed, as opposed to a block change. Renderers can then coalesce the
	// mark into an active build instead of restarting it; see
	// WorldRenderer::markDirtyFromLighting.
	bool isMarkingFromLighting() const { return markingFromLighting; }
	// True while a block change made by the local player (break, place, use)
	// is being applied. RenderGlobal::markBlocksDirty marks the section urgent
	// only inside this scope; every other block change -- a flowing spring,
	// falling gravel, the neighbour's populate -- is streaming work however
	// close to the viewer it lands.
	bool isMarkingFromPlayerEdit() const { return markingFromPlayerEdit; }
	class PlayerEditMarkScope
	{
	public:
		explicit PlayerEditMarkScope(World *w) : world(w), previous(w != nullptr && w->markingFromPlayerEdit)
		{
			if (world != nullptr)
				world->markingFromPlayerEdit = true;
		}
		~PlayerEditMarkScope()
		{
			if (world != nullptr)
				world->markingFromPlayerEdit = previous;
		}
		PlayerEditMarkScope(const PlayerEditMarkScope &) = delete;
		PlayerEditMarkScope &operator=(const PlayerEditMarkScope &) = delete;

	private:
		World *world;
		bool previous;
	};
	// Population lighting is batched per chunk section on low-console profiles.
	// Block callbacks still execute immediately; only flood-fill queue insertion is deferred.
	bool batchPopulationLightingUpdate(EnumSkyBlock *type, int_t minX, int_t minY, int_t minZ,
	                                   int_t maxX, int_t maxY, int_t maxZ);
	void flushPopulationLightingBatches();
	void markBlockNeedsUpdate(int_t i, int_t j, int_t k);
	void notifyBlockChange(int_t i, int_t j, int_t k, int_t l);
	void markBlocksDirtyVertical(int_t i, int_t j, int_t k, int_t l);
	void markBlockAsNeedsUpdate(int_t i, int_t j, int_t k);
	void markBlocksDirty(int_t i, int_t j, int_t k, int_t l, int_t i1, int_t j1);
	void notifyChunkPublishedForRender(int_t chunkX, int_t chunkZ);
	void notifyBlocksOfNeighborChange(int_t i, int_t j, int_t k, int_t l);

private:
	void notifyBlockOfNeighborChange(int_t i, int_t j, int_t k, int_t l);

public:
	bool canBlockSeeTheSky(int_t i, int_t j, int_t k);
	int_t getFullBlockLightValue(int_t i, int_t j, int_t k);
	int_t getBlockLightValue(int_t i, int_t j, int_t k);
	int_t getBlockLightValue_do(int_t i, int_t j, int_t k, bool flag);
	bool canExistingBlockSeeTheSky(int_t i, int_t j, int_t k);
	int_t getHeightValue(int_t i, int_t j);
	int_t getHeight() override;
	bool func_48452_a() override;
	int_t getPrecipitationHeight(int_t x, int_t z);
	int_t getTopSolidOrLiquidBlock(int_t x, int_t z);
	BiomeGenBase *getBiomeGenForCoords(int_t x, int_t z) override;
	bool isBlockHydratedDirectly(int_t x, int_t y, int_t z);
	bool isBlockHydratedIndirectly(int_t x, int_t y, int_t z);
	bool isBlockHydrated(int_t x, int_t y, int_t z, bool requireEdge);
	bool canSnowAt(int_t x, int_t y, int_t z);
	void neighborLightPropagationChanged(EnumSkyBlock *enumskyblock, int_t i, int_t j, int_t k, int_t l);
	int_t getSkyBlockTypeBrightness(EnumSkyBlock *enumskyblock, int_t i, int_t j, int_t k);
	int_t getSavedLightValue(EnumSkyBlock *enumskyblock, int_t i, int_t j, int_t k);
	int_t getLightBrightnessForSkyBlocks(int_t x, int_t y, int_t z, int_t minimumBlockLight) override;
	void setLightValue(EnumSkyBlock *enumskyblock, int_t i, int_t j, int_t k, int_t l);
	float getBrightness(int_t i, int_t j, int_t k, int_t l) override;
	float getLightBrightness(int_t i, int_t j, int_t k) override;
	bool isDaytime();
	MovingObjectPosition *rayTraceBlocks(Vec3D *vec3d, Vec3D *vec3d1);
	MovingObjectPosition *rayTraceBlocks_do(Vec3D *vec3d, Vec3D *vec3d1, bool flag);
	MovingObjectPosition *rayTraceBlocks(Vec3D *start, Vec3D *end, bool stopOnLiquid, bool ignoreNonCollidable);
	MovingObjectPosition *rayTraceBlocks_do_do(Vec3D *start, Vec3D *end, bool stopOnLiquid, bool ignoreNonCollidable);
	MovingObjectPosition *getDoorMaterial(Vec3D *vec3d, Vec3D *vec3d1, bool flag, bool flag1);
	void playSoundAtEntity(Entity *entity, const jstring &s, float f, float f1);
	void playSoundEffect(double d, double d1, double d2, const jstring &s, float f, float f1);
	void playRecord(const jstring &s, int_t i, int_t j, int_t k);
	void spawnParticle(const jstring &s, double d, double d1, double d2, double d3, double d4, double d5);
	bool addWeatherEffect(Entity *entity);
	virtual bool entityJoinedWorld(Entity *entity);
	bool spawnEntityInWorld(Entity *entity) { return entityJoinedWorld(entity); }

protected:
	virtual void obtainEntitySkin(Entity *entity);
	virtual void releaseEntitySkin(Entity *entity);
	void trackLoadedEntityPointer(Entity *entity);
	void untrackLoadedEntityPointer(Entity *entity);
	void rebuildLoadedEntityPointerSet() const;

public:
	virtual void setEntityDead(Entity *entity);
	// Queue an already-dead detached entity for the normal updateEntities()
	// ownership drain. Subclasses use this when a network destroy arrives while
	// the entity is waiting for a chunk and is not in loadedEntityList.
	void queueEntityForDestruction(Entity *entity);
	// C++ port only (Java relied on GC): World owns entities and `delete`s them in
	// updateEntities. Non-owning indexes in subclasses (e.g. WorldClient's
	// entitySpawnQueue / knownEntities / entityHash) must drop the pointer before it
	// is freed, otherwise a stale pointer left in the spawn queue is dereferenced on
	// the next tick (dynamic_cast on freed memory -> access violation). Called right
	// before every `delete entity`. No-op in base World.
	virtual void onEntityRemoved(Entity *entity);
	bool isLoadedEntityPointer(const Entity *entity) const;
	bool isLoadedTileEntityPointer(const TileEntity *tileEntity) const;
	void detachEntityForWorldChange(Entity *entity);
	void addWorldAccess(IWorldAccess *iworldaccess);
	void removeWorldAccess(IWorldAccess *iworldaccess);
	std::vector<AxisAlignedBB *> &getCollidingBoundingBoxes(Entity *entity, AxisAlignedBB *axisalignedbb);
#if PLATFORM_EARLY_COLLISION_EXIT
	bool hasCollidingBoundingBoxes(Entity *entity, AxisAlignedBB *axisalignedbb);
#endif
#if PLATFORM_FLOAT_COLLISION_SWEEP
	// getCollidingBoundingBoxes for Entity::moveEntity: the same block and
	// entity candidates, delivered as float boxes in the sweep's local frame.
	void collectCollisionSweep(Entity *entity, AxisAlignedBB *axisalignedbb, PlatformCollisionSweep &sweep);
#endif
	int_t calculateSkylightSubtracted(float f);
	Vec3D *getBlockLightValue_do(Entity *entity, float f);
	float getCelestialAngle(float f);
	int_t getMoonPhase(float partialTicks);
	float getCelestialAngleRadians(float partialTicks);
	Vec3D *drawClouds(float partialTicks);
	float func_35464_b(float partialTicks);
	Vec3D *getEntityBrightness(float f);
	Vec3D *getFogColor(float f);
	Vec3D *getSkyColor(Entity *entity, float f);   // func_4079_a
	Vec3D *getCloudFogColor(float f);              // func_628_d
	float getRainStrength(float f);
	float getThunderStrength(float f);
	int_t findTopSolidBlock(int_t i, int_t j);
	float getStarBrightness(float f);
	virtual void scheduleBlockUpdate(int_t i, int_t j, int_t k, int_t l, int_t i1);
	void scheduleBlockUpdateFromLoad(int_t x, int_t y, int_t z, int_t blockId, int_t delay);
	std::vector<NextTickListEntry *> getPendingBlockUpdates(Chunk *chunk, bool remove);
	void updateEntities();
	void getPistonMovingProgress(const std::vector<TileEntity *> &collection);
	void updateEntity(Entity *entity);
	void updateEntityWithOptionalForce(Entity *entity, bool flag);
	void ensureEntityChunkRetention(Entity *entity);
	void prefetchEntityChunkRetention(Entity *entity);
	bool checkIfAABBIsClear(AxisAlignedBB *axisalignedbb);
	bool getIsAnyLiquid(AxisAlignedBB *axisalignedbb);
	bool isAnyLiquid(AxisAlignedBB *axisalignedbb) { return getIsAnyLiquid(axisalignedbb); }
	bool isBoundingBoxBurning(AxisAlignedBB *axisalignedbb);
	bool handleMaterialAcceleration(AxisAlignedBB *axisalignedbb, Material *material, Entity *entity);
	bool isMaterialInBB(AxisAlignedBB *axisalignedbb, Material *material);
	bool isAABBInMaterial(AxisAlignedBB *axisalignedbb, Material *material);
	Explosion *createExplosion(Entity *entity, double d, double d1, double d2, float f);
	Explosion *newExplosion(Entity *entity, double d, double d1, double d2, float f, bool flag);
	float getLivingSound(Vec3D *vec3d, AxisAlignedBB *axisalignedbb);
	void onBlockHit(EntityPlayer *entityplayer, int_t i, int_t j, int_t k, int_t l);
	Entity *getSkyBlockType(const std::type_info &class1);
	jstring getHurtSound();
	jstring getContainerItem();
	void addTileEntity(const std::vector<TileEntity *> &tileEntities);
	TileEntity *getBlockTileEntity(int_t i, int_t j, int_t k) override;
	void setBlockTileEntity(int_t i, int_t j, int_t k, TileEntity *tileentity);
	void removeBlockTileEntity(int_t i, int_t j, int_t k);
	bool isBlockOpaqueCube(int_t i, int_t j, int_t k) override;
	bool isBlockNormalCube(int_t i, int_t j, int_t k) override;
	void saveWorldIndirectly(IProgressUpdate *iprogressupdate);
	bool isSafeToSave(int_t counter);
	bool updatingLighting();
	int_t getPendingLightingUpdateCount() const;
	void updateAllLightTypes(int_t x, int_t y, int_t z);
	void updateLightByType(EnumSkyBlock *type, int_t x, int_t y, int_t z);
	void scheduleLightingUpdate(EnumSkyBlock *enumskyblock, int_t i, int_t j, int_t k, int_t l, int_t i1, int_t j1);
	void scheduleLightingUpdate_do(EnumSkyBlock *enumskyblock, int_t i, int_t j, int_t k, int_t l, int_t i1, int_t j1, bool flag);
	void calculateInitialSkylight();
	void setAllowedMobSpawns(bool flag, bool flag1);
	void setNaturalMobSpawningEnabled(bool enabled);
	virtual void tick();

private:
	void initCraftingStats();
	void destroyEntity(Entity *entity);
	Pathfinder *getReusablePathfinder(IBlockAccess *blockAccess, bool openDoors, bool breakDoors,
	                                  bool avoidWater, bool canSwim);

protected:
	virtual void updateWeather();

private:
	void calculateInitialWeather();
	void clearWeather();
	int_t computeSkyLightValue(int_t currentLight, int_t x, int_t y, int_t z, int_t blockId, int_t opacity);
	int_t computeBlockLightValue(int_t currentLight, int_t x, int_t y, int_t z, int_t blockId, int_t opacity);
	void stopPrecipitation();

protected:
	void func_48461_r();
	void func_48458_a(int_t chunkOriginX, int_t chunkOriginZ, Chunk *chunk);
	virtual void updateBlocksAndPlayCaveSounds();

public:
	virtual bool TickUpdates(bool flag);
	virtual bool tickUpdates(bool flag);
	void tickBlocksAndAmbiance();
	void dropOldChunks();
	void randomDisplayUpdates(int_t i, int_t j, int_t k);
	std::vector<Entity *> &getEntitiesWithinAABBExcludingEntity(Entity *entity, AxisAlignedBB *axisalignedbb);
	std::vector<Entity *> getEntitiesWithinAABB(const std::type_info &class1, AxisAlignedBB *axisalignedbb);
	Entity *findNearestEntityWithinAABB(const std::type_info &class1, AxisAlignedBB *axisalignedbb, Entity *excludingEntity);
	Entity *getEntityByID(int_t entityId);
	std::vector<Entity *> &getLoadedEntityList();
	// Java: func_698_b — chunk-modified notification when a TE state changes;
	// fans out to every IWorldAccess listener.
	void markTileEntityChunkModified(int_t i, int_t j, int_t k, TileEntity *tileentity);
	void updateTileEntityChunkAndDoNothing(int_t x, int_t y, int_t z, TileEntity *tileEntity);
	void notifyTileEntityRenderersRemoved(TileEntity *tileentity);
	void detachTileEntityForChunkUnload(TileEntity *tileentity);
	// Not in Java: close any player screen backed by this TileEntity before it
	// is deleted. See the comment on the definition in World.cpp.
	void closeContainersUsing(TileEntity *tileentity);
	// Not in Java: the TileEntity queued for this block by
	// BlockContainer::onBlockAdded but not yet installed in its chunk, if any.
	TileEntity *getPendingTileEntity(int_t x, int_t y, int_t z);
	int_t countEntities(EnumCreatureTypeTag tag);
	void getShadowSize(const std::vector<Entity *> &list);
	void getEntityBrightnessForRender(const std::vector<Entity *> &list);
	void getTextureData();
	bool canBlockBePlacedAt(int_t i, int_t j, int_t k, int_t l, bool flag, int_t i1);
	bool canMineBlock(EntityPlayer *player, int_t x, int_t y, int_t z);
	bool extinguishFire(EntityPlayer *player, int_t x, int_t y, int_t z, int_t side);
	bool func_48457_a(EntityPlayer *player, int_t x, int_t y, int_t z, int_t side);
	Entity *func_4085_a(const std::type_info &classType);
	PathEntity *getPathToEntity(Entity *entity, Entity *entity1, float f);
	PathEntity *getPathToEntity(Entity *entity, Entity *entity1, float f, bool openDoors, bool breakDoors, bool avoidWater, bool canSwim);
	PathEntity *getEntityPathToXYZ(Entity *entity, int_t i, int_t j, int_t k, float f);
	PathEntity *getEntityPathToXYZ(Entity *entity, int_t i, int_t j, int_t k, float f, bool openDoors, bool breakDoors, bool avoidWater, bool canSwim);
	bool isBlockProvidingPowerTo(int_t i, int_t j, int_t k, int_t l);
	bool isBlockGettingPowered(int_t i, int_t j, int_t k);
	bool isBlockIndirectlyProvidingPowerTo(int_t i, int_t j, int_t k, int_t l);
	bool isBlockIndirectlyGettingPowered(int_t i, int_t j, int_t k);
	EntityPlayer *getClosestPlayerToEntity(Entity *entity, double d);
	EntityPlayer *getClosestVulnerablePlayerToEntity(Entity *entity, double distance);
	EntityPlayer *getClosestPlayer(double d, double d1, double d2, double d3);
	EntityPlayer *func_48456_a(double x, double z, double maxDistance);
	EntityPlayer *getPlayerEntityByName(const jstring &s);
	void setChunkData(int_t i, int_t j, int_t k, int_t l, int_t i1, int_t j1, const std::vector<byte_t> &abyte0);
	virtual void sendQuittingDisconnectingPacket();
	void setMapStorage(MapStorage *storage);
	void checkSessionLock();
	void setWorldTime(long_t l);
	long_t getRandomSeed();
	Random &setRandomSeed(int_t x, int_t z, int_t salt);
	long_t getWorldTime();
	ChunkCoordinates getSpawnPoint();
	void setSpawnPoint(ChunkCoordinates *chunkcoordinates);
	void joinEntityInSurroundings(Entity *entity);
	bool loadTexture(EntityPlayer *entityplayer, int_t i, int_t j, int_t k);
	void getEntityLivingSound(Entity *entity, byte_t byte0);
	void updateEntityList();
	IChunkProvider *getIChunkProvider();
	void playNoteAt(int_t i, int_t j, int_t k, int_t l, int_t i1);
	void playAuxSFX(int_t type, int_t x, int_t y, int_t z, int_t data);
	void playAuxSFXAtEntity(EntityPlayer *player, int_t type, int_t x, int_t y, int_t z, int_t data);
	WorldInfo *getWorldInfo();
	void updateAllPlayersSleepingFlag();

protected:
	void wakeUpAllPlayers();

public:
	bool isAllPlayersFullyAsleep();
	float initToolsStats(float f);
	float initBlockStats(float f);
	void registerStat(float f);
	bool initStats();
	bool initItemStats();
	bool canBlockBeRainedOn(int_t i, int_t j, int_t k);
	bool canLightningStrikeAt(int_t x, int_t y, int_t z);
	void setItemData(const jstring &s, MapDataBase *mapdatabase);
	MapDataBase *loadItemData(const std::type_info &class1, const jstring &s);
	MapDataBase *loadItemData(const jstring &s, MapDataFactory factory);
	int_t getUniqueDataId(const jstring &s);
	void isTrapdoorOpen(int_t i, int_t j, int_t k, int_t l, int_t i1);
	void getTrapdoorFacing(EntityPlayer *entityplayer, int_t i, int_t j, int_t k, int_t l, int_t i1);

	bool scheduledUpdatesAreImmediate;

private:
	std::vector<MetadataChunkBlock> lightingToUpdate;
	// Single-cell jobs currently in lightingToUpdate; see LightingQueueCellSet.
	LightingQueueCellSet lightingQueuedCells;
	// Render marks accumulated while updatingLighting() drains the queue.
	LightingDirtyRegions lightingDirtyRegions;

	static bool isSingleCellLightingJob(const MetadataChunkBlock &job)
	{
		return job.minX == job.maxX && job.minY == job.maxY && job.minZ == job.maxZ;
	}

public:
	std::vector<Entity *> loadedEntityList;
	bool entityCountsDirty = true;
	int_t countedMonsters = 0;
	int_t countedCreatures = 0;
	int_t countedWaterCreatures = 0;

private:
#if PLATFORM_PC_LEGACY
	mutable std::unordered_set<const Entity *> loadedEntityPointerSet;
#elif PLATFORM_PS2
	mutable Ps2EntityPointerIndex loadedEntityPointerIndex;
	mutable bool loadedEntityPointerIndexOverflow = false;
#endif
	std::vector<Entity *> unloadedEntityList;
	std::set<NextTickListEntry *, NextTickListEntryComparator> scheduledTickTreeSet;
	std::unordered_set<NextTickListEntry *, NextTickListEntryHash, NextTickListEntryEqual> scheduledTickSet;
	JavaHashSet<NextTickListEntry *, NextTickListEntryHash, NextTickListEntryEqual> scheduledTickOrder;
#if PLATFORM_PC_LEGACY
	PcLegacyTickScheduler *pcLegacyTickScheduler = nullptr;
#endif

public:
	std::vector<TileEntity *> loadedTileEntityList;

private:
	std::vector<TileEntity *> entityID;

public:
	std::vector<EntityPlayer *> playerEntities;
	std::vector<Entity *> weatherEffects;

private:
	long_t maxDamage;

public:
	int_t skylightSubtracted;

protected:
	int_t itemName;
	const int_t itemIconIndex = 0x3c6ef35f;
	// Java defaults these to 0.0F. In C++ they MUST be initialized: if left as garbage that
	// happens to be NaN, updateWeather()'s clamps (NaN<0 and NaN>1 are both false) never fix it,
	// so getRainStrengthInterpolated/getThunderStrengthInterpolated return NaN -> calculateSkylightSubtracted's (int)(f2*11) becomes
	// INT_MIN -> getBlockLightValue underflows to 0 everywhere -> hostile mobs spawn in daylight and
	// flowers/crops drop (canBlockStay sees light 0). Root cause of the [SPAWN-DBG]/[PLANT-DROP] bugs.
	float prevRainingStrength = 0.0f;
	float rainingStrength = 0.0f;
	float prevThunderingStrength = 0.0f;
	float thunderingStrength = 0.0f;
	int_t chatMessage;

public:
	int_t chatColor;
	bool editingBlocks;
	VillageCollection *villageCollectionObj = nullptr;
	VillageSiege *villageSiegeObj = nullptr;

private:
	long_t lockTimestamp;

protected:
	int_t autosavePeriod;

public:
	int_t difficultySetting = 0;

private:
	// Java initializes updateLCG from a temporary Random before constructing
	// world.rand. Declaration order preserves that construction sequence.
	int_t updateLCG = Random().nextInt();

public:
	Random rand;
#if PLATFORM_REUSE_RANDOM_DISPLAY_RNG
	// Display-only RNG. A fixed explicit seed avoids Random()'s clock/atomic
	// seeding path while World::rand still selects the vanilla probe positions.
	Random randomDisplayRandom{0x4d43504cLL};
#endif
	bool isNewWorld;
	WorldProvider *worldProvider;

protected:
	std::vector<IWorldAccess *> worldAccesses;
	IChunkProvider *chunkProvider;
	ISaveHandler *saveHandler;
	WorldInfo *worldInfo;

public:
	bool findingSpawnPoint = false;

private:
	bool allPlayersSleeping = false;

public:
	MapStorage *worldType;

private:
	std::vector<AxisAlignedBB *> collidingBoundingBoxes;
	bool pistonExtending;
	int_t lightingUpdatesCounter;
	bool spawnHostileMobs;
	bool spawnPeacefulMobs;
	bool naturalMobSpawningEnabled = true;
	static int_t lightingUpdatesScheduled;

protected:
	JavaHashSet<ChunkCoordIntPair, ChunkCoordIntPairValueHash, ChunkCoordIntPairValueEqual> positionsToUpdate;
#if PLATFORM_CACHE_RANDOM_TICK_CHUNKS
	std::vector<std::uint64_t> positionsToUpdatePlayerChunkKeys;
	std::vector<ChunkCoordIntPair> positionsToUpdateOrder;
	bool positionsToUpdateCacheValid = false;
#endif

private:
	int_t soundCounter;
	std::vector<Entity *> maxStackSize;

public:
	bool multiplayerWorld;

	// Lightning flash timer (field_27172_i in obfuscated Java)
	int_t field_27172_i;

	void setEntityState(Entity *entity, byte_t byte0);        // func_9425_a – fan out entity-state event to clients
	float getBlockDensity(Vec3D *vec, AxisAlignedBB *aabb);   // func_675_a – explosion ray-sampling
	void addLoadedEntities(const std::vector<Entity *> &list);       // bulk-add entities to loadedEntityList
	virtual void unloadEntities(const std::vector<Entity *> &list); // bulk-queue entities for unload
	void addLoadedTileEntities(const std::vector<TileEntity *> &collection);
	bool isThundering();                                       // true if rainingStrength > 0.9
	bool isRaining();                                       // true if rainingStrength > 0.2
	float getRainStrengthInterpolated(float partialTicks);                    // internal: raw rain lerp
	float getThunderStrengthInterpolated(float partialTicks);                    // internal: thunder * rain
	void setRainStrength(float strength);                         // set rain strength instantly

	// Methods implemented in World.cpp but missing from header
	Entity *findEntityByClass(const std::type_info &classType);
	std::string getLoadedEntityStats();
	std::string getChunkProviderStats();
	int_t getLoadedChunkCount() const;
	void unloadAllChunks();
	void initializeWeatherStrengths();

private:
	Chunk *getPopulationFastChunk(int_t x, int_t y, int_t z) const;
	std::array<int_t, 32768> lightUpdateBlockList{};
	std::vector<TileEntity *> tileEntitiesToAdd;
	std::vector<TileEntity *> tileEntitiesToRemove;
	bool populationFastPathActive = false;
	int_t populationFastBaseX = 0;
	int_t populationFastBaseZ = 0;
	Chunk *populationFastChunks[4] = {};
	PopulationRegionAccessor populationRegionAccessor;
	ChunkLocalDecorationTarget chunkLocalDecoration;
	bool markingFromLighting = false;
	bool markingFromPlayerEdit = false;
	struct PopulationLightingBatch
	{
		bool valid = false;
		int_t minX = 0;
		int_t minY = 0;
		int_t minZ = 0;
		int_t maxX = 0;
		int_t maxY = 0;
		int_t maxZ = 0;
	};
	static constexpr int_t POPULATION_LIGHTING_TYPE_COUNT = 2;
	static constexpr int_t POPULATION_LIGHTING_CHUNK_COUNT = 4;
	std::array<PopulationLightingBatch,
	           POPULATION_LIGHTING_TYPE_COUNT * POPULATION_LIGHTING_CHUNK_COUNT * WorldHeight::SECTION_COUNT>
		populationLightingBatches{};
	long_t field_1019_F;
	int_t thunderTime;
	std::vector<Entity *> entitiesWithinAABBExcludingEntity;
	std::vector<Entity *> entitiesWithinAABB;
	MapStorage *mapStorage;
	bool ownsSaveHandler = true;
	bool ownsMapStorage = true;
	bool updatingTileEntities = false;
	Pathfinder *pathfinderCache = nullptr;
};
