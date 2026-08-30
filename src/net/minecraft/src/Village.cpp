#include "Village.h"

#include <algorithm>
#include <cmath>
#include <limits>

#include "AxisAlignedBB.h"
#include "Block.h"
#include "Entity.h"
#include "EntityList.h"
#include "EntityLiving.h"
#include "EntityIronGolem.h"
#include "EntityVillager.h"
#include "MathHelper.h"
#include "java/Arithmetic.h"
#include "java/Math.h"
#include "VillageDoorInfo.h"
#include "World.h"

namespace
{
int_t javaAbsInt(int_t value)
{
	return value < 0 ? JavaArithmetic::intSub(0, value) : value;
}
}

Village::Village(World *world)
	: worldObj(world), centerHelper(0, 0, 0), center(0, 0, 0), villageRadius(0),
	  lastAddDoorTimestamp(0), tickCounter(0), numVillagers(0), numIronGolems(0)
{
}

Village::~Village() = default;

void Village::tick(int_t tick)
{
	tickCounter = tick;
	removeDeadAndOutOfRangeDoors();
	removeDeadAndOldAgressors();
	if (tick % 20 == 0)
		updateNumVillagers();
	if (tick % 30 == 0)
		updateNumIronGolems();
	int_t desiredGolems = numVillagers / 16;
	if (numIronGolems < desiredGolems && villageDoorInfoList.size() > 20 && worldObj != nullptr && worldObj->rand.nextInt(7000) == 0)
		trySpawnIronGolem();
}

ChunkCoordinates Village::getCenter() const { return ChunkCoordinates(&center); }
int_t Village::getVillageRadius() const { return villageRadius; }
int_t Village::getNumVillageDoors() const { return (int_t)villageDoorInfoList.size(); }
int_t Village::getTicksSinceLastDoorAdding() const { return JavaArithmetic::intSub(tickCounter, lastAddDoorTimestamp); }
int_t Village::getNumVillagers() const { return numVillagers; }

bool Village::isInRange(int_t x, int_t y, int_t z) const
{
	return center.getDistanceSquared(x, y, z) < (float)JavaArithmetic::intMul(villageRadius, villageRadius);
}

const std::vector<std::unique_ptr<VillageDoorInfo>> &Village::getVillageDoorInfoList() const
{
	return villageDoorInfoList;
}

VillageDoorInfo *Village::findNearestDoor(int_t x, int_t y, int_t z) const
{
	VillageDoorInfo *best = nullptr;
	int_t bestDistance = std::numeric_limits<int_t>::max();
	for (const auto &door : villageDoorInfoList)
	{
		int_t distance = door->getDistanceSquared(x, y, z);
		if (distance < bestDistance)
		{
			best = door.get();
			bestDistance = distance;
		}
	}
	return best;
}

VillageDoorInfo *Village::findNearestDoorUnrestricted(int_t x, int_t y, int_t z) const
{
	VillageDoorInfo *best = nullptr;
	int_t bestScore = std::numeric_limits<int_t>::max();
	for (const auto &door : villageDoorInfoList)
	{
		int_t distance = door->getDistanceSquared(x, y, z);
		int_t score = distance > 256 ? JavaArithmetic::intMul(distance, 1000) : door->getDoorOpeningRestrictionCounter();
		if (score < bestScore)
		{
			best = door.get();
			bestScore = score;
		}
	}
	return best;
}

VillageDoorInfo *Village::getVillageDoorAt(int_t x, int_t y, int_t z) const
{
	if (center.getDistanceSquared(x, y, z) > (float)JavaArithmetic::intMul(villageRadius, villageRadius))
		return nullptr;
	for (const auto &door : villageDoorInfoList)
	{
		if (door->posX == x && door->posZ == z && javaAbsInt(JavaArithmetic::intSub(door->posY, y)) <= 1)
			return door.get();
	}
	return nullptr;
}

void Village::addVillageDoorInfo(VillageDoorInfo *door)
{
	if (door == nullptr)
		return;
	centerHelper.x = JavaArithmetic::intAdd(centerHelper.x, door->posX);
	centerHelper.y = JavaArithmetic::intAdd(centerHelper.y, door->posY);
	centerHelper.z = JavaArithmetic::intAdd(centerHelper.z, door->posZ);
	lastAddDoorTimestamp = door->lastActivityTimestamp;
	villageDoorInfoList.emplace_back(door);
	updateVillageRadiusAndCenter();
}

bool Village::isAnnihilated() const
{
	return villageDoorInfoList.empty();
}

void Village::addOrRenewAgressor(EntityLiving *aggressor)
{
	if (aggressor == nullptr)
		return;
	for (VillageAgressor &entry : villageAgressors)
	{
		if (entry.matches(aggressor))
		{
			entry.agressionTime = tickCounter;
			return;
		}
	}
	villageAgressors.emplace_back(this, aggressor, tickCounter);
}

EntityLiving *Village::findNearestVillageAggressor(EntityLiving *target) const
{
	if (target == nullptr)
		return nullptr;
	double bestDistance = std::numeric_limits<double>::max();
	EntityLiving *best = nullptr;
	for (const VillageAgressor &entry : villageAgressors)
	{
		EntityLiving *aggressor = entry.resolve(worldObj);
		if (aggressor == nullptr)
			continue;
		double distance = aggressor->getDistanceSqToEntity(target);
		if (distance <= bestDistance)
		{
			bestDistance = distance;
			best = aggressor;
		}
	}
	return best;
}

void Village::onEntityRemoved(Entity *entity)
{
	villageAgressors.erase(std::remove_if(villageAgressors.begin(), villageAgressors.end(),
		[entity](const VillageAgressor &entry) { return entity != nullptr && entry.entityId == entity->entityId; }), villageAgressors.end());
}

void Village::updateNumIronGolems()
{
	if (worldObj == nullptr)
	{
		numIronGolems = 0;
		return;
	}

	AxisAlignedBB *bounds = AxisAlignedBB::getBoundingBoxFromPool(
		static_cast<double>(JavaArithmetic::intSub(center.x, villageRadius)),
		static_cast<double>(JavaArithmetic::intSub(center.y, 4)),
		static_cast<double>(JavaArithmetic::intSub(center.z, villageRadius)),
		static_cast<double>(JavaArithmetic::intAdd(center.x, villageRadius)),
		static_cast<double>(JavaArithmetic::intAdd(center.y, 4)),
		static_cast<double>(JavaArithmetic::intAdd(center.z, villageRadius)));
	const std::vector<Entity *> &golems = worldObj->getEntitiesWithinAABB(typeid(EntityIronGolem), bounds);
	numIronGolems = static_cast<int_t>(golems.size());
}

void Village::updateNumVillagers()
{
	if (worldObj == nullptr)
	{
		numVillagers = 0;
		return;
	}

	AxisAlignedBB *bounds = AxisAlignedBB::getBoundingBoxFromPool(
		static_cast<double>(JavaArithmetic::intSub(center.x, villageRadius)),
		static_cast<double>(JavaArithmetic::intSub(center.y, 4)),
		static_cast<double>(JavaArithmetic::intSub(center.z, villageRadius)),
		static_cast<double>(JavaArithmetic::intAdd(center.x, villageRadius)),
		static_cast<double>(JavaArithmetic::intAdd(center.y, 4)),
		static_cast<double>(JavaArithmetic::intAdd(center.z, villageRadius)));
	const std::vector<Entity *> &villagers = worldObj->getEntitiesWithinAABB(typeid(EntityVillager), bounds);
	numVillagers = static_cast<int_t>(villagers.size());
}

void Village::removeDeadAndOldAgressors()
{
	villageAgressors.erase(std::remove_if(villageAgressors.begin(), villageAgressors.end(),
		[this](const VillageAgressor &entry)
		{
			EntityLiving *aggressor = entry.resolve(worldObj);
			return aggressor == nullptr || !aggressor->isEntityAlive() ||
				javaAbsInt(JavaArithmetic::intSub(tickCounter, entry.agressionTime)) > 300;
		}), villageAgressors.end());
}

void Village::removeDeadAndOutOfRangeDoors()
{
	bool changed = false;
	bool resetCounters = worldObj != nullptr && worldObj->rand.nextInt(50) == 0;
	for (auto it = villageDoorInfoList.begin(); it != villageDoorInfoList.end();)
	{
		VillageDoorInfo *door = it->get();
		if (resetCounters)
			door->resetDoorOpeningRestrictionCounter();
		if (isBlockDoor(door->posX, door->posY, door->posZ) &&
			javaAbsInt(JavaArithmetic::intSub(tickCounter, door->lastActivityTimestamp)) <= 1200)
		{
			++it;
			continue;
		}
		centerHelper.x = JavaArithmetic::intSub(centerHelper.x, door->posX);
		centerHelper.y = JavaArithmetic::intSub(centerHelper.y, door->posY);
		centerHelper.z = JavaArithmetic::intSub(centerHelper.z, door->posZ);
		door->isDetachedFromVillageFlag = true;
		it = villageDoorInfoList.erase(it);
		changed = true;
	}
	if (changed)
		updateVillageRadiusAndCenter();
}

bool Village::isBlockDoor(int_t x, int_t y, int_t z) const
{
	return worldObj != nullptr && Block::doorWood != nullptr && worldObj->getBlockId(x, y, z) == Block::doorWood->blockID;
}

void Village::updateVillageRadiusAndCenter()
{
	int_t count = (int_t)villageDoorInfoList.size();
	if (count == 0)
	{
		center.set(0, 0, 0);
		villageRadius = 0;
		return;
	}
	center.set(centerHelper.x / count, centerHelper.y / count, centerHelper.z / count);
	int_t maxDistance = 0;
	for (const auto &door : villageDoorInfoList)
		maxDistance = std::max(maxDistance, door->getDistanceSquared(center.x, center.y, center.z));
	const int_t distanceRadius = JavaArithmetic::intAdd(
		JavaArithmetic::doubleToInt(JavaMath::sqrt((double)maxDistance)), 1);
	villageRadius = std::max(32, distanceRadius);
}

bool Village::trySpawnIronGolem()
{
	if (worldObj == nullptr)
		return false;
	const int_t centerX = MathHelper::floor_float((float)center.x);
	const int_t centerY = MathHelper::floor_float((float)center.y);
	const int_t centerZ = MathHelper::floor_float((float)center.z);
	for (int_t attempt = 0; attempt < 10; ++attempt)
	{
		int_t x = JavaArithmetic::intSub(JavaArithmetic::intAdd(centerX, worldObj->rand.nextInt(16)), 8);
		int_t y = JavaArithmetic::intSub(JavaArithmetic::intAdd(centerY, worldObj->rand.nextInt(6)), 3);
		int_t z = JavaArithmetic::intSub(JavaArithmetic::intAdd(centerZ, worldObj->rand.nextInt(16)), 8);
		if (!isInRange(x, y, z) || !isValidIronGolemSpawningLocation(x, y, z, 2, 4, 2))
			continue;
		Entity *golem = EntityList::createEntity(99, worldObj);
		if (golem == nullptr)
			return false;
		golem->setPosition((double)x, (double)y, (double)z);
		if (!worldObj->entityJoinedWorld(golem))
		{
			delete golem;
			return false;
		}
		++numIronGolems;
		return true;
	}
	return false;
}

bool Village::isValidIronGolemSpawningLocation(int_t x, int_t y, int_t z, int_t sizeX, int_t sizeY, int_t sizeZ) const
{
	if (!worldObj->isBlockNormalCube(x, JavaArithmetic::intSub(y, 1), z))
		return false;
	int_t minX = JavaArithmetic::intSub(x, sizeX / 2);
	int_t minZ = JavaArithmetic::intSub(z, sizeZ / 2);
	const int_t maxX = JavaArithmetic::intAdd(minX, sizeX);
	const int_t maxY = JavaArithmetic::intAdd(y, sizeY);
	const int_t maxZ = JavaArithmetic::intAdd(minZ, sizeZ);
	for (int_t bx = minX; bx < maxX; bx = JavaArithmetic::intAdd(bx, 1))
		for (int_t by = y; by < maxY; by = JavaArithmetic::intAdd(by, 1))
			for (int_t bz = minZ; bz < maxZ; bz = JavaArithmetic::intAdd(bz, 1))
				if (worldObj->isBlockNormalCube(bx, by, bz))
					return false;
	return true;
}
