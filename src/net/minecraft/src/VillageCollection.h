#pragma once

#include <memory>
#include <vector>

#include "ChunkCoordinates.h"
#include "java/Type.h"

class Entity;
class Village;
class VillageDoorInfo;
class World;

// net.minecraft.src.VillageCollection
class VillageCollection
{
public:
	explicit VillageCollection(World *world);
	~VillageCollection();

	void addVillagerPosition(int_t x, int_t y, int_t z);
	void tick();
	const std::vector<std::unique_ptr<Village>> &getVillageList() const;
	Village *findNearestVillage(int_t x, int_t y, int_t z, int_t radius) const;
	void onEntityRemoved(Entity *entity);

private:
	void removeAnnihilatedVillages();
	void dropOldestVillagerPosition();
	void addNewDoorsToVillageOrCreateVillage();
	void addUnassignedWoodenDoorsAroundToNewDoorsList(const ChunkCoordinates &position);
	VillageDoorInfo *getVillageDoorAt(int_t x, int_t y, int_t z) const;
	void addDoorToNewListIfAppropriate(int_t x, int_t y, int_t z);
	bool isVillagerPositionPresent(int_t x, int_t y, int_t z) const;
	bool isWoodenDoorAt(int_t x, int_t y, int_t z) const;

	World *worldObj;
	std::vector<ChunkCoordinates> villagerPositionsList;
	std::vector<std::unique_ptr<VillageDoorInfo>> newDoors;
	std::vector<std::unique_ptr<Village>> villageList;
	int_t tickCounter;
};
