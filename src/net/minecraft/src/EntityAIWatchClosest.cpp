#include "EntityAIWatchClosest.h"

#include "AxisAlignedBB.h"
#include "Entity.h"
#include "EntityLiving.h"
#include "EntityLookHelper.h"
#include "EntityPlayer.h"
#include "World.h"

EntityAIWatchClosest::EntityAIWatchClosest(EntityLiving *entity, const std::type_info &type, float distance)
	: EntityAIWatchClosest(entity, type, distance, 0.02f)
{
}

EntityAIWatchClosest::EntityAIWatchClosest(EntityLiving *entity, const std::type_info &type, float distance, float chance)
	: watcher(entity), closestEntityId(-1), maxDistance(distance), lookTime(0), chance(chance), targetType(&type)
{
	setMutexBits(2);
}

Entity *EntityAIWatchClosest::resolveClosestEntity() const
{
	if (watcher == nullptr || watcher->worldObj == nullptr || closestEntityId < 0)
		return nullptr;
	return watcher->worldObj->getEntityByID(closestEntityId);
}

bool EntityAIWatchClosest::matchesTargetType(Entity *entity) const
{
	return entity != nullptr && targetType != nullptr && entity->isAssignableTo(*targetType);
}

bool EntityAIWatchClosest::shouldExecute()
{
	closestEntityId = -1;
	if (watcher == nullptr || watcher->worldObj == nullptr || watcher->getRNG().nextFloat() >= chance)
		return false;
	Entity *closestEntity = nullptr;
	if (targetType != nullptr && *targetType == typeid(EntityPlayer))
		closestEntity = watcher->worldObj->getClosestPlayerToEntity(watcher, (double)maxDistance);
	else if (targetType != nullptr)
		closestEntity = watcher->worldObj->findNearestEntityWithinAABB(*targetType,
			watcher->boundingBox->expand((double)maxDistance, 3.0, (double)maxDistance), watcher);
	if (closestEntity == nullptr || !matchesTargetType(closestEntity))
		return false;
	closestEntityId = closestEntity->entityId;
	return true;
}

bool EntityAIWatchClosest::continueExecuting()
{
	Entity *closestEntity = resolveClosestEntity();
	return closestEntity != nullptr && closestEntity->isEntityAlive() &&
		watcher != nullptr && watcher->getDistanceSqToEntity(closestEntity) <= (double)(maxDistance * maxDistance) && lookTime > 0;
}

void EntityAIWatchClosest::startExecuting()
{
	if (watcher != nullptr)
		lookTime = 40 + watcher->getRNG().nextInt(40);
}

void EntityAIWatchClosest::resetTask()
{
	closestEntityId = -1;
}

void EntityAIWatchClosest::updateTask()
{
	Entity *closestEntity = resolveClosestEntity();
	if (watcher == nullptr || closestEntity == nullptr || watcher->getLookHelper() == nullptr)
		return;
	watcher->getLookHelper()->setLookPosition(closestEntity->posX, closestEntity->posY + (double)closestEntity->getEyeHeight(),
		closestEntity->posZ, 10.0f, (float)watcher->getVerticalFaceSpeed());
	--lookTime;
}
