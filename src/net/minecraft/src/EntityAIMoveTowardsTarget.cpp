#include "EntityAIMoveTowardsTarget.h"

#include "EntityCreature.h"
#include "EntityLiving.h"
#include "PathNavigate.h"
#include "RandomPositionGenerator.h"
#include "Vec3D.h"
#include "World.h"

EntityAIMoveTowardsTarget::EntityAIMoveTowardsTarget(EntityCreature *entity, float moveSpeed, float maxDistance)
	: theEntity(entity), targetEntityId(-1), movePosX(0.0), movePosY(0.0), movePosZ(0.0),
	  speed(moveSpeed), maxTargetDistance(maxDistance)
{
	setMutexBits(1);
}

EntityLiving *EntityAIMoveTowardsTarget::resolveTarget() const
{
	if (theEntity == nullptr || theEntity->worldObj == nullptr || targetEntityId < 0)
		return nullptr;
	return dynamic_cast<EntityLiving *>(theEntity->worldObj->getEntityByID(targetEntityId));
}

bool EntityAIMoveTowardsTarget::shouldExecute()
{
	targetEntityId = -1;
	if (theEntity == nullptr)
		return false;
	EntityLiving *target = theEntity->getAttackTarget();
	if (target == nullptr || target->getDistanceSqToEntity(theEntity) > (double)(maxTargetDistance * maxTargetDistance))
		return false;
	Vec3D *position = RandomPositionGenerator::findRandomTargetBlockTowards(theEntity, 16, 7,
		Vec3D::createVector(target->posX, target->posY, target->posZ));
	if (position == nullptr)
		return false;
	movePosX = position->xCoord;
	movePosY = position->yCoord;
	movePosZ = position->zCoord;
	targetEntityId = target->entityId;
	return true;
}

bool EntityAIMoveTowardsTarget::continueExecuting()
{
	EntityLiving *target = resolveTarget();
	return theEntity != nullptr && theEntity->getNavigator() != nullptr && !theEntity->getNavigator()->noPath() &&
		target != nullptr && target->isEntityAlive() && target->getDistanceSqToEntity(theEntity) < (double)(maxTargetDistance * maxTargetDistance);
}

void EntityAIMoveTowardsTarget::startExecuting()
{
	if (theEntity != nullptr && theEntity->getNavigator() != nullptr)
		theEntity->getNavigator()->tryMoveToXYZ(movePosX, movePosY, movePosZ, speed);
}

void EntityAIMoveTowardsTarget::resetTask()
{
	targetEntityId = -1;
}
