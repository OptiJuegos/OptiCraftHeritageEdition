#include "EntityAIMoveIndoors.h"

#include "EntityCreature.h"
#include "MathHelper.h"
#include "PathNavigate.h"
#include "RandomPositionGenerator.h"
#include "Vec3D.h"
#include "Village.h"
#include "VillageCollection.h"
#include "VillageDoorInfo.h"
#include "World.h"
#include "WorldProvider.h"

EntityAIMoveIndoors::EntityAIMoveIndoors(EntityCreature *entity)
	: entityObj(entity), doorInsideX(0), doorInsideY(0), doorInsideZ(0), insidePosX(-1), insidePosZ(-1)
{
	setMutexBits(1);
}

bool EntityAIMoveIndoors::shouldExecute()
{
	if (entityObj == nullptr || entityObj->worldObj == nullptr || entityObj->worldObj->worldProvider == nullptr)
		return false;
	World *world = entityObj->worldObj;
	if ((world->isDaytime() && !world->isRaining()) || world->worldProvider->hasNoSky)
		return false;
	if (entityObj->getRNG().nextInt(50) != 0)
		return false;
	if (insidePosX != -1 && entityObj->getDistanceSq((double)insidePosX, entityObj->posY, (double)insidePosZ) < 4.0)
		return false;
	if (world->villageCollectionObj == nullptr)
		return false;
	Village *village = world->villageCollectionObj->findNearestVillage(
		MathHelper::floor_double(entityObj->posX), MathHelper::floor_double(entityObj->posY), MathHelper::floor_double(entityObj->posZ), 14);
	if (village == nullptr)
		return false;
	VillageDoorInfo *doorInfo = village->findNearestDoorUnrestricted(MathHelper::floor_double(entityObj->posX), MathHelper::floor_double(entityObj->posY), MathHelper::floor_double(entityObj->posZ));
	if (doorInfo == nullptr)
		return false;
	doorInsideX = doorInfo->getInsidePosX();
	doorInsideY = doorInfo->getInsidePosY();
	doorInsideZ = doorInfo->getInsidePosZ();
	return true;
}

bool EntityAIMoveIndoors::continueExecuting()
{
	return entityObj != nullptr && entityObj->getNavigator() != nullptr && !entityObj->getNavigator()->noPath();
}

void EntityAIMoveIndoors::startExecuting()
{
	insidePosX = -1;
	if (entityObj == nullptr || entityObj->getNavigator() == nullptr)
		return;
	if (entityObj->getDistanceSq((double)doorInsideX, (double)doorInsideY, (double)doorInsideZ) > 256.0)
	{
		Vec3D *inside = Vec3D::createVector((double)doorInsideX + 0.5, (double)doorInsideY, (double)doorInsideZ + 0.5);
		Vec3D *target = RandomPositionGenerator::findRandomTargetBlockTowards(entityObj, 14, 3, inside);
		if (target != nullptr)
			entityObj->getNavigator()->tryMoveToXYZ(target->xCoord, target->yCoord, target->zCoord, 0.3f);
	}
	else
	{
		entityObj->getNavigator()->tryMoveToXYZ((double)doorInsideX + 0.5, (double)doorInsideY, (double)doorInsideZ + 0.5, 0.3f);
	}
}

void EntityAIMoveIndoors::resetTask()
{
	insidePosX = doorInsideX;
	insidePosZ = doorInsideZ;
}
