#include "EntityAIRestrictOpenDoor.h"

#include "EntityCreature.h"
#include "MathHelper.h"
#include "PathNavigate.h"
#include "Village.h"
#include "VillageCollection.h"
#include "VillageDoorInfo.h"
#include "World.h"

EntityAIRestrictOpenDoor::EntityAIRestrictOpenDoor(EntityCreature *entity) : entityObj(entity), doorX(0), doorY(0), doorZ(0), hasDoor(false)
{
}

VillageDoorInfo *EntityAIRestrictOpenDoor::resolveFrontDoor() const
{
	if (!hasDoor || entityObj == nullptr || entityObj->worldObj == nullptr || entityObj->worldObj->villageCollectionObj == nullptr)
		return nullptr;
	Village *village = entityObj->worldObj->villageCollectionObj->findNearestVillage(doorX, doorY, doorZ, 32);
	return village != nullptr ? village->getVillageDoorAt(doorX, doorY, doorZ) : nullptr;
}

bool EntityAIRestrictOpenDoor::shouldExecute()
{
	hasDoor = false;
	if (entityObj == nullptr || entityObj->worldObj == nullptr || entityObj->worldObj->isDaytime() || entityObj->worldObj->villageCollectionObj == nullptr)
		return false;
	Village *village = entityObj->worldObj->villageCollectionObj->findNearestVillage(
		MathHelper::floor_double(entityObj->posX), MathHelper::floor_double(entityObj->posY), MathHelper::floor_double(entityObj->posZ), 16);
	if (village == nullptr)
		return false;
	VillageDoorInfo *frontDoor = village->findNearestDoor(MathHelper::floor_double(entityObj->posX), MathHelper::floor_double(entityObj->posY), MathHelper::floor_double(entityObj->posZ));
	if (frontDoor == nullptr || (double)frontDoor->getInsideDistanceSquare(MathHelper::floor_double(entityObj->posX),
		MathHelper::floor_double(entityObj->posY), MathHelper::floor_double(entityObj->posZ)) >= 2.25)
		return false;
	doorX = frontDoor->posX;
	doorY = frontDoor->posY;
	doorZ = frontDoor->posZ;
	hasDoor = true;
	return true;
}

bool EntityAIRestrictOpenDoor::continueExecuting()
{
	VillageDoorInfo *frontDoor = resolveFrontDoor();
	return entityObj != nullptr && frontDoor != nullptr && !entityObj->worldObj->isDaytime() &&
		!frontDoor->isDetachedFromVillageFlag && frontDoor->isInside(MathHelper::floor_double(entityObj->posX), MathHelper::floor_double(entityObj->posZ));
}

void EntityAIRestrictOpenDoor::startExecuting()
{
	if (entityObj != nullptr && entityObj->getNavigator() != nullptr)
	{
		entityObj->getNavigator()->setBreakDoors(false);
		entityObj->getNavigator()->setEnterDoors(false);
	}
}

void EntityAIRestrictOpenDoor::resetTask()
{
	if (entityObj != nullptr && entityObj->getNavigator() != nullptr)
	{
		entityObj->getNavigator()->setBreakDoors(true);
		entityObj->getNavigator()->setEnterDoors(true);
	}
	hasDoor = false;
}

void EntityAIRestrictOpenDoor::updateTask()
{
	VillageDoorInfo *frontDoor = resolveFrontDoor();
	if (frontDoor != nullptr)
		frontDoor->incrementDoorOpeningRestrictionCounter();
}
