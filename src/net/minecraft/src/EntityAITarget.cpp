#include "EntityAITarget.h"

#include "EntityLiving.h"
#include "EntityPlayer.h"
#include "EntitySenses.h"
#include "EntityTameable.h"
#include "MathHelper.h"
#include "PathEntity.h"
#include "PathNavigate.h"
#include "PathPoint.h"
#include "PlayerCapabilities.h"
#include "java/Arithmetic.h"

EntityAITarget::EntityAITarget(EntityLiving *owner, float distance, bool checkSight)
	: EntityAITarget(owner, distance, checkSight, false)
{
}

EntityAITarget::EntityAITarget(EntityLiving *owner, float distance, bool checkSight, bool onlyNearby)
	: taskOwner(owner), targetDistance(distance), shouldCheckSight(checkSight), nearbyOnly(onlyNearby),
	  targetSearchStatus(0), targetSearchDelay(0), unseenTicks(0)
{
}

bool EntityAITarget::continueExecuting()
{
	if (taskOwner == nullptr)
		return false;
	EntityLiving *target = taskOwner->getAttackTarget();
	if (target == nullptr || !target->isEntityAlive())
		return false;
	if (taskOwner->getDistanceSqToEntity(target) > (double)(targetDistance * targetDistance))
		return false;
	if (shouldCheckSight)
	{
		EntitySenses *senses = taskOwner->getEntitySenses();
		if (senses == nullptr || !senses->canSee(target))
		{
			if (++unseenTicks > 60)
				return false;
		}
		else
		{
			unseenTicks = 0;
		}
	}
	return true;
}

void EntityAITarget::startExecuting()
{
	targetSearchStatus = 0;
	targetSearchDelay = 0;
	unseenTicks = 0;
}

void EntityAITarget::resetTask()
{
	if (taskOwner != nullptr)
		taskOwner->setAttackTarget(nullptr);
}

bool EntityAITarget::isSuitableTarget(EntityLiving *target, bool includeInvulnerablePlayers)
{
	if (taskOwner == nullptr || target == nullptr || target == taskOwner || !target->isEntityAlive())
		return false;
	if (target->boundingBox == nullptr || taskOwner->boundingBox == nullptr ||
		target->boundingBox->maxY <= taskOwner->boundingBox->minY || target->boundingBox->minY >= taskOwner->boundingBox->maxY)
		return false;
	if (!taskOwner->canAttackEntity(target))
		return false;
	EntityTameable *tameableOwner = dynamic_cast<EntityTameable *>(taskOwner);
	if (tameableOwner != nullptr && tameableOwner->isTamed())
	{
		EntityTameable *tameableTarget = dynamic_cast<EntityTameable *>(target);
		if (tameableTarget != nullptr && tameableTarget->isTamed())
			return false;
		if (target == tameableOwner->getOwner())
			return false;
	}
	else
	{
		EntityPlayer *player = dynamic_cast<EntityPlayer *>(target);
		if (player != nullptr && !includeInvulnerablePlayers && player->capabilities.disableDamage)
			return false;
	}
	if (!taskOwner->isWithinHomeDistance(MathHelper::floor_double(target->posX), MathHelper::floor_double(target->posY), MathHelper::floor_double(target->posZ)))
		return false;
	if (shouldCheckSight)
	{
		EntitySenses *senses = taskOwner->getEntitySenses();
		if (senses == nullptr || !senses->canSee(target))
			return false;
	}
	if (nearbyOnly)
	{
		if (--targetSearchDelay <= 0)
			targetSearchStatus = 0;
		if (targetSearchStatus == 0)
			targetSearchStatus = canEasilyReach(target) ? 1 : 2;
		if (targetSearchStatus == 2)
			return false;
	}
	return true;
}

bool EntityAITarget::func_48376_a(EntityLiving *target, bool includeInvulnerablePlayers)
{
	return isSuitableTarget(target, includeInvulnerablePlayers);
}

bool EntityAITarget::canEasilyReach(EntityLiving *target)
{
	if (taskOwner == nullptr || target == nullptr || taskOwner->getNavigator() == nullptr)
		return false;
	targetSearchDelay = 10 + taskOwner->getRNG().nextInt(5);
	PathEntity *path = taskOwner->getNavigator()->getPathToEntityLiving(target);
	if (path == nullptr)
		return false;
	PathPoint *finalPoint = path->getFinalPathPoint();
	bool reachable = false;
	if (finalPoint != nullptr)
	{
		int_t dx = JavaArithmetic::intSub(finalPoint->xCoord, MathHelper::floor_double(target->posX));
		int_t dz = JavaArithmetic::intSub(finalPoint->zCoord, MathHelper::floor_double(target->posZ));
		int_t distanceSquared = JavaArithmetic::intAdd(
			JavaArithmetic::intMul(dx, dx), JavaArithmetic::intMul(dz, dz));
		reachable = (double)distanceSquared <= 2.25;
	}
	delete path;
	return reachable;
}
