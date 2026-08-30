#include "EntityAIAvoidEntity.h"

#include "AxisAlignedBB.h"
#include "Entity.h"
#include "EntityCreature.h"
#include "EntityPlayer.h"
#include "EntitySenses.h"
#include "EntityTameable.h"
#include "PathEntity.h"
#include "PathNavigate.h"
#include "RandomPositionGenerator.h"
#include "Vec3D.h"
#include "World.h"

EntityAIAvoidEntity::EntityAIAvoidEntity(EntityCreature *entity, const std::type_info &type, float distance, float slowSpeed, float fastSpeed)
	: theEntity(entity), targetType(&type), farSpeed(slowSpeed), nearSpeed(fastSpeed), targetEntityId(-1),
	  avoidDistance(distance), path(nullptr), navigator(entity != nullptr ? entity->getNavigator() : nullptr)
{
	setMutexBits(1);
}

EntityAIAvoidEntity::~EntityAIAvoidEntity()
{
	delete path;
}

Entity *EntityAIAvoidEntity::resolveTarget() const
{
	if (theEntity == nullptr || theEntity->worldObj == nullptr || targetEntityId < 0)
		return nullptr;
	return theEntity->worldObj->getEntityByID(targetEntityId);
}

bool EntityAIAvoidEntity::shouldExecute()
{
	targetEntityId = -1;
	delete path;
	path = nullptr;
	if (theEntity == nullptr || theEntity->worldObj == nullptr || navigator == nullptr)
		return false;
	Entity *target = nullptr;
	if (targetType != nullptr && *targetType == typeid(EntityPlayer))
	{
		EntityTameable *tameable = dynamic_cast<EntityTameable *>(theEntity);
		if (tameable != nullptr && tameable->isTamed())
			return false;
		target = theEntity->worldObj->getClosestPlayerToEntity(theEntity, (double)avoidDistance);
	}
	else
	{
		const std::vector<Entity *> &entities = theEntity->worldObj->getEntitiesWithinAABB(*targetType,
			theEntity->boundingBox->expand((double)avoidDistance, 3.0, (double)avoidDistance));
		if (!entities.empty())
			target = entities.front();
	}
	if (target == nullptr || target->isDead || !theEntity->getEntitySenses()->canSee(target))
		return false;
	Vec3D *targetPosition = Vec3D::createVector(target->posX, target->posY, target->posZ);
	Vec3D *away = RandomPositionGenerator::findRandomTargetBlockAwayFrom(theEntity, 16, 7, targetPosition);
	if (away == nullptr || target->getDistanceSq(away->xCoord, away->yCoord, away->zCoord) < target->getDistanceSqToEntity(theEntity))
		return false;
	path = navigator->getPathToXYZ(away->xCoord, away->yCoord, away->zCoord);
	if (path == nullptr || !path->isDestinationSame(away))
	{
		delete path;
		path = nullptr;
		return false;
	}
	targetEntityId = target->entityId;
	return true;
}

bool EntityAIAvoidEntity::continueExecuting()
{
	return navigator != nullptr && !navigator->noPath();
}

void EntityAIAvoidEntity::startExecuting()
{
	if (navigator != nullptr && path != nullptr)
	{
		navigator->setPath(path, farSpeed);
		path = nullptr;
	}
}

void EntityAIAvoidEntity::resetTask()
{
	if (path != nullptr)
	{
		delete path;
		path = nullptr;
	}
	targetEntityId = -1;
}

void EntityAIAvoidEntity::updateTask()
{
	Entity *target = resolveTarget();
	if (theEntity == nullptr || navigator == nullptr || target == nullptr)
		return;
	navigator->setSpeed(theEntity->getDistanceSqToEntity(target) < 49.0 ? nearSpeed : farSpeed);
}
