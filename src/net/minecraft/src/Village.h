#pragma once

#include <memory>
#include <vector>

#include "ChunkCoordinates.h"
#include "VillageAgressor.h"
#include "java/Type.h"

class Entity;
class EntityLiving;
class VillageDoorInfo;
class World;

// net.minecraft.src.Village
class Village
{
public:
	explicit Village(World *world);
	~Village();

	void tick(int_t tickCounter);
	ChunkCoordinates getCenter() const;
	int_t getVillageRadius() const;
	int_t getNumVillageDoors() const;
	int_t getTicksSinceLastDoorAdding() const;
	int_t getNumVillagers() const;
	bool isInRange(int_t x, int_t y, int_t z) const;
	const std::vector<std::unique_ptr<VillageDoorInfo>> &getVillageDoorInfoList() const;
	VillageDoorInfo *findNearestDoor(int_t x, int_t y, int_t z) const;
	VillageDoorInfo *findNearestDoorUnrestricted(int_t x, int_t y, int_t z) const;
	VillageDoorInfo *getVillageDoorAt(int_t x, int_t y, int_t z) const;
	void addVillageDoorInfo(VillageDoorInfo *door);
	bool isAnnihilated() const;
	void addOrRenewAgressor(EntityLiving *aggressor);
	EntityLiving *findNearestVillageAggressor(EntityLiving *target) const;
	void onEntityRemoved(Entity *entity);

private:

	void updateNumIronGolems();
	void updateNumVillagers();
	void removeDeadAndOldAgressors();
	void removeDeadAndOutOfRangeDoors();
	bool isBlockDoor(int_t x, int_t y, int_t z) const;
	void updateVillageRadiusAndCenter();
	bool trySpawnIronGolem();
	bool isValidIronGolemSpawningLocation(int_t x, int_t y, int_t z, int_t sizeX, int_t sizeY, int_t sizeZ) const;

	World *worldObj;
	std::vector<std::unique_ptr<VillageDoorInfo>> villageDoorInfoList;
	ChunkCoordinates centerHelper;
	ChunkCoordinates center;
	int_t villageRadius;
	int_t lastAddDoorTimestamp;
	int_t tickCounter;
	int_t numVillagers;
	std::vector<VillageAgressor> villageAgressors;
	int_t numIronGolems;
};
