#include "EntityAIMoveThroughVillage.h"

#include <limits>

#include "EntityCreature.h"
#include "MathHelper.h"
#include "PathEntity.h"
#include "PathNavigate.h"
#include "RandomPositionGenerator.h"
#include "Vec3D.h"
#include "Village.h"
#include "VillageCollection.h"
#include "VillageDoorInfo.h"
#include "World.h"

EntityAIMoveThroughVillage::EntityAIMoveThroughVillage(EntityCreature *entity, float moveSpeed, bool avoidDay)
	: theEntity(entity), speed(moveSpeed), path(nullptr), doorPosition(0, 0, 0), hasDoor(false), noMovementAtDay(avoidDay)
{
	setMutexBits(1);
}

EntityAIMoveThroughVillage::~EntityAIMoveThroughVillage()
{
	delete path;
}

bool EntityAIMoveThroughVillage::shouldExecute()
{
	delete path;
	path = nullptr;
	hasDoor = false;
	trimDoorList();
	if (theEntity == nullptr || theEntity->worldObj == nullptr || theEntity->getNavigator() == nullptr)
		return false;
	if (noMovementAtDay && theEntity->worldObj->isDaytime())
		return false;
	VillageCollection *collection = theEntity->worldObj->villageCollectionObj;
	if (collection == nullptr)
		return false;
	Village *village = collection->findNearestVillage(MathHelper::floor_double(theEntity->posX),
		MathHelper::floor_double(theEntity->posY), MathHelper::floor_double(theEntity->posZ), 0);
	if (village == nullptr || !selectDoor(village, doorPosition))
		return false;
	hasDoor = true;

	PathNavigate *navigator = theEntity->getNavigator();
	bool canBreakDoors = navigator->getCanBreakDoors();
	navigator->setBreakDoors(false);
	path = navigator->getPathToXYZ((double)doorPosition.x, (double)doorPosition.y, (double)doorPosition.z);
	navigator->setBreakDoors(canBreakDoors);
	if (path != nullptr)
		return true;

	Vec3D *doorVec = Vec3D::createVector((double)doorPosition.x, (double)doorPosition.y, (double)doorPosition.z);
	Vec3D *towards = RandomPositionGenerator::findRandomTargetBlockTowards(theEntity, 10, 7, doorVec);
	if (towards == nullptr)
		return false;
	navigator->setBreakDoors(false);
	path = navigator->getPathToXYZ(towards->xCoord, towards->yCoord, towards->zCoord);
	navigator->setBreakDoors(canBreakDoors);
	return path != nullptr;
}

bool EntityAIMoveThroughVillage::continueExecuting()
{
	if (theEntity == nullptr || theEntity->getNavigator() == nullptr || !hasDoor || theEntity->getNavigator()->noPath())
		return false;
	float distance = theEntity->width + 4.0f;
	return theEntity->getDistanceSq((double)doorPosition.x, (double)doorPosition.y, (double)doorPosition.z) > (double)(distance * distance);
}

void EntityAIMoveThroughVillage::startExecuting()
{
	if (theEntity != nullptr && theEntity->getNavigator() != nullptr && path != nullptr)
	{
		theEntity->getNavigator()->setPath(path, speed);
		path = nullptr;
	}
}

void EntityAIMoveThroughVillage::resetTask()
{
	delete path;
	path = nullptr;
	if (theEntity != nullptr && hasDoor && (theEntity->getNavigator()->noPath() ||
		theEntity->getDistanceSq((double)doorPosition.x, (double)doorPosition.y, (double)doorPosition.z) < 16.0))
		recentDoors.push_back(doorPosition);
	hasDoor = false;
}

bool EntityAIMoveThroughVillage::selectDoor(Village *village, ChunkCoordinates &result) const
{
	if (village == nullptr || theEntity == nullptr)
		return false;
	VillageDoorInfo *best = nullptr;
	int_t bestDistance = std::numeric_limits<int_t>::max();
	for (const auto &ownedDoor : village->getVillageDoorInfoList())
	{
		VillageDoorInfo *door = ownedDoor.get();
		if (door == nullptr || isDoorRemembered(door))
			continue;
		int_t distance = door->getDistanceSquared(MathHelper::floor_double(theEntity->posX),
			MathHelper::floor_double(theEntity->posY), MathHelper::floor_double(theEntity->posZ));
		if (distance < bestDistance)
		{
			best = door;
			bestDistance = distance;
		}
	}
	if (best == nullptr)
		return false;
	result.set(best->posX, best->posY, best->posZ);
	return true;
}

bool EntityAIMoveThroughVillage::isDoorRemembered(const VillageDoorInfo *door) const
{
	if (door == nullptr)
		return false;
	for (const ChunkCoordinates &position : recentDoors)
	{
		if (position.x == door->posX && position.y == door->posY && position.z == door->posZ)
			return true;
	}
	return false;
}

void EntityAIMoveThroughVillage::trimDoorList()
{
	if (recentDoors.size() > 15)
		recentDoors.erase(recentDoors.begin());
}
