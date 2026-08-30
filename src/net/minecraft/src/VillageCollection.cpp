#include "VillageCollection.h"

#include <cmath>
#include <limits>

#include "Block.h"
#include "BlockDoor.h"
#include "Entity.h"
#include "Village.h"
#include "VillageDoorInfo.h"
#include "World.h"
#include "java/Arithmetic.h"

VillageCollection::VillageCollection(World *world) : worldObj(world), tickCounter(0)
{
}

VillageCollection::~VillageCollection() = default;

void VillageCollection::addVillagerPosition(int_t x, int_t y, int_t z)
{
	if (villagerPositionsList.size() <= 64 && !isVillagerPositionPresent(x, y, z))
		villagerPositionsList.emplace_back(x, y, z);
}

void VillageCollection::tick()
{
	tickCounter = JavaArithmetic::intAdd(tickCounter, 1);
	for (const auto &village : villageList)
		village->tick(tickCounter);
	removeAnnihilatedVillages();
	dropOldestVillagerPosition();
	addNewDoorsToVillageOrCreateVillage();
}

const std::vector<std::unique_ptr<Village>> &VillageCollection::getVillageList() const
{
	return villageList;
}

Village *VillageCollection::findNearestVillage(int_t x, int_t y, int_t z, int_t radius) const
{
	Village *best = nullptr;
	float bestDistance = std::numeric_limits<float>::max();
	for (const auto &village : villageList)
	{
		ChunkCoordinates center = village->getCenter();
		float distance = center.getDistanceSquared(x, y, z);
		if (distance < bestDistance)
		{
			const int_t allowed = JavaArithmetic::intAdd(radius, village->getVillageRadius());
			if (distance <= static_cast<float>(JavaArithmetic::intMul(allowed, allowed)))
			{
				best = village.get();
				bestDistance = distance;
			}
		}
	}
	return best;
}

void VillageCollection::onEntityRemoved(Entity *entity)
{
	for (const auto &village : villageList)
		village->onEntityRemoved(entity);
}

void VillageCollection::removeAnnihilatedVillages()
{
	for (auto it = villageList.begin(); it != villageList.end();)
	{
		if ((*it)->isAnnihilated())
			it = villageList.erase(it);
		else
			++it;
	}
}

void VillageCollection::dropOldestVillagerPosition()
{
	if (villagerPositionsList.empty())
		return;
	ChunkCoordinates position = villagerPositionsList.front();
	villagerPositionsList.erase(villagerPositionsList.begin());
	addUnassignedWoodenDoorsAroundToNewDoorsList(position);
}

void VillageCollection::addNewDoorsToVillageOrCreateVillage()
{
	for (auto &ownedDoor : newDoors)
	{
		VillageDoorInfo *door = ownedDoor.get();
		bool assigned = false;
		for (const auto &village : villageList)
		{
			ChunkCoordinates center = village->getCenter();
			int_t distance = JavaArithmetic::doubleToInt(
				center.getEuclideanDistanceTo(door->posX, door->posY, door->posZ));
			if (distance <= JavaArithmetic::intAdd(32, village->getVillageRadius()))
			{
				village->addVillageDoorInfo(ownedDoor.release());
				assigned = true;
				break;
			}
		}
		if (!assigned)
		{
			std::unique_ptr<Village> village(new Village(worldObj));
			village->addVillageDoorInfo(ownedDoor.release());
			villageList.push_back(std::move(village));
		}
	}
	newDoors.clear();
}

void VillageCollection::addUnassignedWoodenDoorsAroundToNewDoorsList(const ChunkCoordinates &position)
{
	const int_t minX = JavaArithmetic::intSub(position.x, 16);
	const int_t maxX = JavaArithmetic::intAdd(position.x, 16);
	const int_t minY = JavaArithmetic::intSub(position.y, 4);
	const int_t maxY = JavaArithmetic::intAdd(position.y, 4);
	const int_t minZ = JavaArithmetic::intSub(position.z, 16);
	const int_t maxZ = JavaArithmetic::intAdd(position.z, 16);
	for (int_t x = minX; x < maxX; ++x)
		for (int_t y = minY; y < maxY; ++y)
			for (int_t z = minZ; z < maxZ; ++z)
			{
				if (!isWoodenDoorAt(x, y, z))
					continue;
				VillageDoorInfo *door = getVillageDoorAt(x, y, z);
				if (door == nullptr)
					addDoorToNewListIfAppropriate(x, y, z);
				else
					door->lastActivityTimestamp = tickCounter;
			}
}

VillageDoorInfo *VillageCollection::getVillageDoorAt(int_t x, int_t y, int_t z) const
{
	for (const auto &door : newDoors)
		if (door->posX == x && door->posZ == z && JavaArithmetic::intAbs(JavaArithmetic::intSub(door->posY, y)) <= 1)
			return door.get();
	for (const auto &village : villageList)
	{
		VillageDoorInfo *door = village->getVillageDoorAt(x, y, z);
		if (door != nullptr)
			return door;
	}
	return nullptr;
}

void VillageCollection::addDoorToNewListIfAppropriate(int_t x, int_t y, int_t z)
{
	if (worldObj == nullptr || Block::doorWood == nullptr)
		return;
	BlockDoor *doorBlock = dynamic_cast<BlockDoor *>(Block::doorWood);
	if (doorBlock == nullptr)
		return;
	int_t orientation = doorBlock->getDoorOrientation(worldObj, x, y, z);
	int_t skyScore = 0;
	if (orientation != 0 && orientation != 2)
	{
		for (int_t offset = -5; offset < 0; ++offset)
			if (worldObj->canBlockSeeTheSky(x, y, JavaArithmetic::intAdd(z, offset))) --skyScore;
		for (int_t offset = 1; offset <= 5; ++offset)
			if (worldObj->canBlockSeeTheSky(x, y, JavaArithmetic::intAdd(z, offset))) ++skyScore;
		if (skyScore != 0)
			newDoors.emplace_back(new VillageDoorInfo(x, y, z, 0, skyScore > 0 ? -2 : 2, tickCounter));
	}
	else
	{
		for (int_t offset = -5; offset < 0; ++offset)
			if (worldObj->canBlockSeeTheSky(JavaArithmetic::intAdd(x, offset), y, z)) --skyScore;
		for (int_t offset = 1; offset <= 5; ++offset)
			if (worldObj->canBlockSeeTheSky(JavaArithmetic::intAdd(x, offset), y, z)) ++skyScore;
		if (skyScore != 0)
			newDoors.emplace_back(new VillageDoorInfo(x, y, z, skyScore > 0 ? -2 : 2, 0, tickCounter));
	}
}

bool VillageCollection::isVillagerPositionPresent(int_t x, int_t y, int_t z) const
{
	for (const ChunkCoordinates &position : villagerPositionsList)
		if (position.x == x && position.y == y && position.z == z)
			return true;
	return false;
}

bool VillageCollection::isWoodenDoorAt(int_t x, int_t y, int_t z) const
{
	return worldObj != nullptr && Block::doorWood != nullptr && worldObj->getBlockId(x, y, z) == Block::doorWood->blockID;
}
