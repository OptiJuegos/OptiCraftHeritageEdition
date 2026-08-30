#pragma once

#include <cstddef>
#include <cstdint>
#include <typeinfo>
#include <unordered_map>
#include <vector>

#include "java/HashSet.h"
#include "java/Random.h"
#include "java/Type.h"
#include "ChunkPosition.h"
#include "EnumSkyBlock.h"
#include "WorldHeight.h"

class AxisAlignedBB;
class BlockContainer;
class ExtendedBlockStorage;
class IChunkProvider;
class BiomeGenBase;
class WorldChunkManager;
class ChunkCoordIntPair;
class TileEntity;
class World;
class Entity;

struct ChunkPositionHash
{
	size_t operator()(const ChunkPosition &p) const { return (size_t)(uint_t)p.hashCode(); }
};

struct ChunkPositionEqual
{
	bool operator()(const ChunkPosition &a, const ChunkPosition &b) const { return a.equals(b); }
};

// net.minecraft.src.Chunk
class Chunk
{
public:
	static constexpr int_t SECTION_COUNT = WorldHeight::SECTION_COUNT;
	static constexpr int_t SECTION_HEIGHT = WorldHeight::SECTION_HEIGHT;
	static constexpr int_t WORLD_HEIGHT = WorldHeight::HEIGHT;

	Chunk(World *world, int_t i, int_t j);
	Chunk(World *world, std::vector<byte_t> abyte0, int_t i, int_t j);
	virtual ~Chunk();

	virtual bool isAtLocation(int_t i, int_t j);
	virtual int_t getHeightValue(int_t i, int_t j);
	virtual int_t getTopFilledSegment() const;
	void func_4143_d();
	ExtendedBlockStorage **getBlockStorageArray();
	ExtendedBlockStorage *const *getBlockStorageArray() const;
	void setStorageArrays(ExtendedBlockStorage *const *storage, int_t count = SECTION_COUNT);
	ExtendedBlockStorage *getBlockStorage(int_t sectionY);
	const ExtendedBlockStorage *getBlockStorage(int_t sectionY) const;

	virtual void initBlockLight();
	virtual void generateHeightMap();
	virtual void generateSkylightMap();
	virtual void onChunkLoadData();

	virtual int_t getBlockLightOpacity(int_t i, int_t j, int_t k);
	virtual int_t getBlockID(int_t i, int_t j, int_t k);
	virtual bool setBlockIDWithMetadata(int_t i, int_t j, int_t k, int_t l, int_t i1);
	virtual bool setBlockID(int_t i, int_t j, int_t k, int_t l);
	bool replaceBlockIDForPopulation(int_t i, int_t j, int_t k,
	                                 int_t expectedId, int_t newId);
	virtual int_t getBlockMetadata(int_t i, int_t j, int_t k);
	virtual bool setBlockMetadata(int_t i, int_t j, int_t k, int_t l);
	std::uint32_t getBlockSectionRevision(int_t sectionY) const;

	virtual int_t getSavedLightValue(EnumSkyBlock *enumskyblock, int_t i, int_t j, int_t k);
	virtual void setLightValue(EnumSkyBlock *enumskyblock, int_t i, int_t j, int_t k, int_t l);
	virtual int_t getBlockLightValue(int_t i, int_t j, int_t k, int_t l);

	virtual void addEntity(Entity *entity);
	virtual void removeEntity(Entity *entity);
	virtual void removeEntityAtIndex(Entity *entity, int_t i);
	void removeEntityFromAllSections(Entity *entity);
	virtual bool canBlockSeeTheSky(int_t i, int_t j, int_t k);

	virtual TileEntity *getChunkBlockTileEntity(int_t i, int_t j, int_t k);
	virtual void addTileEntity(TileEntity *tileentity);
	virtual void setChunkBlockTileEntity(int_t i, int_t j, int_t k, TileEntity *tileentity);
	virtual void removeChunkBlockTileEntity(int_t i, int_t j, int_t k);

	virtual void onChunkLoad();
	virtual void onChunkUnload();
	virtual void setChunkModified();
	void markRuntimeSaveRequired();
	void clearRuntimeSaveRequired();
	bool isRuntimeSaveRequired() const;

	virtual void getEntitiesWithinAABBForEntity(Entity *entity, AxisAlignedBB *axisalignedbb, std::vector<Entity *> &list);
	virtual void getEntitiesOfTypeWithinAAAB(const std::type_info &type, AxisAlignedBB *axisalignedbb, std::vector<Entity *> &list);

	virtual bool needsSaving(bool flag);

	// Beta protocol compatibility path. The 1.2.5 protocol uses section masks and
	// func_48494_a() below; keeping this method lets the current multiplayer code
	// remain usable while Packet51 is migrated separately.
	virtual int_t setChunkData(byte_t *abyte0, int_t i, int_t j, int_t k,
	                           int_t l, int_t i1, int_t j1, int_t k1);

	// Minecraft 1.2.5 section payload loader (Packet51/52 layout). Returns false
	// when the supplied buffer is shorter than the masks require and leaves data
	// already copied into earlier sections untouched, matching stream semantics
	// while preventing out-of-bounds reads in C++.
	bool func_48494_a(const byte_t *bytes, std::size_t length,
	                  int_t primaryBitMask, int_t addBitMask,
	                  bool includeBiomeData, std::size_t *bytesConsumed = nullptr);

	virtual bool getAreLevelsEmpty(int_t minY, int_t maxY) const;
	void removeUnknownBlocks();
	void populateChunk(IChunkProvider *provider, IChunkProvider *generator, int_t chunkX, int_t chunkZ);
	int_t getPrecipitationHeight(int_t localX, int_t localZ);
	ChunkCoordIntPair getChunkCoordIntPair() const;
	BiomeGenBase *func_48490_a(int_t localX, int_t localZ, WorldChunkManager *manager);
	void updateSkylight();
	void resetRelightChecks();
	void enqueueRelightChecks();

	std::vector<byte_t> &getBiomeArray();
	const std::vector<byte_t> &getBiomeArray() const;
	bool setBiomeArray(std::vector<byte_t> biomeData);

	Random getRandomWithSeed(long_t l);
	virtual bool isEmpty();
	virtual bool isEmptyChunk();
	void remapBlocks();

	static bool isLit;

	bool field_50120_o;
	bool isChunkLoaded;
	World *worldObj;
	int_t precipitationHeightMap[256];
	bool updateSkylightColumns[256];
	int_t heightMap[256];
	const int_t xPosition;
	const int_t zPosition;
	std::unordered_map<ChunkPosition, TileEntity *, ChunkPositionHash, ChunkPositionEqual> chunkTileEntityMap;
	JavaHashSet<ChunkPosition, ChunkPositionHash, ChunkPositionEqual> chunkTileEntityOrder;
	std::vector<Entity *> entities[SECTION_COUNT];
	bool isTerrainPopulated;
	bool isModified;
	bool runtimeSaveRequired;
	// Set instead of calling generateSkylightMap() immediately when the write
	// happens inside World's population fast path (see
	// World::isPopulationFastPathChunk). One chunk decoration step can place
	// dozens of blocks above the existing heightmap (a tree's trunk and leaves,
	// almost all of them), each of which used to trigger its own full 16x16
	// column rescan; endPopulationFastPath() now collapses those into one.
	bool skylightRegenPending = false;
	bool neverSave;
	bool hasEntities;
	long_t lastSaveTime;
	long_t lastAccessTick;
	std::uint32_t blockSectionRevision[SECTION_COUNT];

private:
	ExtendedBlockStorage *ensureBlockStorage(int_t sectionY);
	void clearBlockStorage(int_t sectionY);
	void propagateSkylightOcclusion(int_t i, int_t j);
	void updateSkylight_do();
	void checkSkylightNeighborHeight(int_t i, int_t j, int_t k);
	void updateSkylightNeighborHeight(int_t i, int_t j, int_t minY, int_t maxY);
	void relightBlock(int_t i, int_t j, int_t k);
	bool isValidLocalPosition(int_t x, int_t y, int_t z) const;

	ExtendedBlockStorage *storageArrays[SECTION_COUNT];
	std::vector<byte_t> blockBiomeArray;
	bool initialHeightMapReady;
	bool isGapLightingUpdated;
	int_t queuedLightChecks;
};
