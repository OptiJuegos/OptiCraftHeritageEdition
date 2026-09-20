#pragma once

#include "World.h"
#include "java/HashSet.h"
#include <unordered_map>
#include <vector>

class ChunkProviderClient;
class MCHash;
class NetClientHandler;
class WorldBlockPositionType;
class WorldSettings;

#include "WorldClientEntityIdentity.h"

class WorldClient : public World
{
public:
	WorldClient(NetClientHandler *sendQueue, long_t seed, int_t dimension);
	WorldClient(NetClientHandler *sendQueue, const WorldSettings &settings, int_t dimension, int_t difficulty);
	~WorldClient() override;

	void tick() override;
	void invalidateBlockReceiveRegion(int_t minX, int_t minY, int_t minZ,
	                                  int_t maxX, int_t maxY, int_t maxZ);
	void setSpawnLocation() override;
	void scheduleBlockUpdate(int_t x, int_t y, int_t z, int_t blockId, int_t delay) override;
	bool TickUpdates(bool force) override;
	void doPreChunk(int_t chunkX, int_t chunkZ, bool load);
	bool shouldKeepChunk(int_t chunkX, int_t chunkZ) const;
	void cacheCompressedChunk(int_t chunkX, int_t chunkZ, bool includeInitialize,
	                          int_t primaryMask, int_t addMask,
	                          std::vector<byte_t> compressed);
	void deferBlockChange(int_t x, int_t y, int_t z, int_t blockId, int_t metadata);
	void prioritizePlayerChunk(int_t chunkX, int_t chunkZ);
	std::size_t getDeferredChunkCount() const { return deferredChunks.size(); }
	std::size_t getDeferredChunkBytes() const { return deferredChunkBytes; }
	ulong_t getDeferredChunkEvictions() const { return deferredChunkEvictions; }
	ulong_t getDeferredChunkBudgetOverflows() const { return deferredChunkBudgetOverflows; }
	std::size_t getDeferredPromotionPendingCount() const;
	ulong_t getDeferredChunkPromotions() const { return deferredChunkPromotions; }
	ulong_t getDeferredChunkCorruptions() const { return deferredChunkCorruptions; }
	std::size_t getPendingEntitySpawnCount() const { return entitySpawnQueue.size(); }
	std::size_t getKnownEntityCount() const { return knownEntities.size(); }
	ulong_t getDeferredEntityChunkPromotions() const { return deferredEntityChunkPromotions; }
	bool entityJoinedWorld(Entity *entity) override;
	void setEntityDead(Entity *entity) override;
	void unloadEntities(const std::vector<Entity *> &list) override;
	void addEntityToWorld(int_t entityId, Entity *entity);
	void applyNetworkPosition(Entity *entity, double x, double y, double z, float yaw, float pitch);
	Entity *getEntityByID(int_t entityId);
	Entity *removeEntityFromWorld(int_t entityId);
	bool setBlockMetadata(int_t x, int_t y, int_t z, int_t metadata) override;
	bool setBlockAndMetadata(int_t x, int_t y, int_t z, int_t blockId, int_t metadata) override;
	bool setBlock(int_t x, int_t y, int_t z, int_t blockId) override;
	bool setBlockAndMetadataAndInvalidate(int_t x, int_t y, int_t z, int_t blockId, int_t metadata);
	void sendQuittingDisconnectingPacket() override;
	void setRainStrength(float strength) { rainingStrength = strength; }

protected:
	IChunkProvider *getChunkProvider() override;
	void updateBlocksAndPlayCaveSounds() override;
	void obtainEntitySkin(Entity *entity) override;
	void releaseEntitySkin(Entity *entity) override;
	void onEntityRemoved(Entity *entity) override;
	void updateWeather() override;

private:
	void trimClientChunkCache();
	void promoteDeferredChunks(const std::vector<Entity *> *priorityEntities = nullptr);
	bool promoteDeferredChunk(int_t chunkX, int_t chunkZ);
	void enforceDeferredChunkBudget();
	void forgetDeferredChunk(int_t chunkX, int_t chunkZ);

	struct DeferredBlockChange
	{
		int_t x, y, z, blockId, metadata;
	};
	struct DeferredMapUpdate
	{
		int_t primaryMask = 0, addMask = 0;
		std::vector<byte_t> compressed;
	};
	struct DeferredChunk
	{
		int_t chunkX = 0, chunkZ = 0;
		int_t primaryMask = 0, addMask = 0;
		std::vector<byte_t> compressed;
		std::vector<DeferredMapUpdate> sectionUpdates;
		std::vector<DeferredBlockChange> changes;
		ulong_t stamp = 0;
	};
	std::unordered_map<ulong_t, DeferredChunk> deferredChunks;
	std::size_t deferredChunkBytes = 0;
	std::size_t deferredBlockChangeCount = 0;
	ulong_t deferredChunkStamp = 0;
	ulong_t deferredChunkEvictions = 0;
	ulong_t deferredChunkBudgetOverflows = 0;
	bool deferredChunkBudgetExceeded = false;
	ulong_t deferredChunkPromotions = 0;
	ulong_t deferredChunkCorruptions = 0;
	ulong_t deferredEntityChunkPromotions = 0;
	std::size_t entityRetryCursor = 0;
	std::vector<WorldBlockPositionType *> pendingBlockChanges;
	NetClientHandler *sendQueue;
	ChunkProviderClient *clientChunkProvider;
	MCHash *entityHash;
	JavaHashSet<Entity *, WorldClientEntityHash, WorldClientEntityEqual> knownEntities;
	JavaHashSet<Entity *, WorldClientEntityHash, WorldClientEntityEqual> entitySpawnQueue;
};
