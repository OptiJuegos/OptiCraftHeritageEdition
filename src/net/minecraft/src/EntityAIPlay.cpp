#include "EntityAIPlay.h"

#include <limits>

#include "AxisAlignedBB.h"
#include "Entity.h"
#include "EntityVillager.h"
#include "PathNavigate.h"
#include "RandomPositionGenerator.h"
#include "Vec3D.h"
#include "World.h"

EntityAIPlay::EntityAIPlay(EntityVillager *villager, float moveSpeed)
	: villagerObj(villager), targetEntityId(-1), speed(moveSpeed), playTime(0)
{
	setMutexBits(1);
}

EntityVillager *EntityAIPlay::resolveTarget() const
{
	if (villagerObj == nullptr || villagerObj->worldObj == nullptr || targetEntityId < 0)
		return nullptr;
	return dynamic_cast<EntityVillager *>(villagerObj->worldObj->getEntityByID(targetEntityId));
}

bool EntityAIPlay::shouldExecute()
{
	targetEntityId = -1;
	if (villagerObj == nullptr || villagerObj->worldObj == nullptr || villagerObj->getGrowingAge() >= 0)
		return false;
	if (villagerObj->getRNG().nextInt(400) != 0)
		return false;

	const std::vector<Entity *> &entities = villagerObj->worldObj->getEntitiesWithinAABB(typeid(EntityVillager),
		villagerObj->boundingBox->expand(6.0, 3.0, 6.0));
	double bestDistance = std::numeric_limits<double>::max();
	for (Entity *entity : entities)
	{
		EntityVillager *candidate = dynamic_cast<EntityVillager *>(entity);
		if (candidate == nullptr || candidate == villagerObj || candidate->isDead || candidate->getIsPlayingFlag() || candidate->getGrowingAge() >= 0)
			continue;
		double distance = candidate->getDistanceSqToEntity(villagerObj);
		if (distance <= bestDistance)
		{
			bestDistance = distance;
			targetEntityId = candidate->entityId;
		}
	}

	if (targetEntityId < 0)
		return RandomPositionGenerator::findRandomTarget(villagerObj, 16, 3) != nullptr;
	return true;
}

bool EntityAIPlay::continueExecuting()
{
	return villagerObj != nullptr && !villagerObj->isDead && playTime > 0;
}

void EntityAIPlay::startExecuting()
{
	if (villagerObj == nullptr)
		return;
	if (resolveTarget() != nullptr)
		villagerObj->setIsPlayingFlag(true);
	playTime = 1000;
}

void EntityAIPlay::resetTask()
{
	if (villagerObj != nullptr)
		villagerObj->setIsPlayingFlag(false);
	targetEntityId = -1;
	playTime = 0;
}

void EntityAIPlay::updateTask()
{
	if (villagerObj == nullptr || villagerObj->getNavigator() == nullptr)
		return;
	--playTime;
	EntityVillager *target = resolveTarget();
	if (target != nullptr)
	{
		if (villagerObj->getDistanceSqToEntity(target) > 4.0)
			villagerObj->getNavigator()->tryMoveToEntityLiving(target, speed);
	}
	else if (villagerObj->getNavigator()->noPath())
	{
		Vec3D *position = RandomPositionGenerator::findRandomTarget(villagerObj, 16, 3);
		if (position != nullptr)
			villagerObj->getNavigator()->tryMoveToXYZ(position->xCoord, position->yCoord, position->zCoord, speed);
	}
}
