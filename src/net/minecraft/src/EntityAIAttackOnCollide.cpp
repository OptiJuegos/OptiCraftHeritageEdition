#include "EntityAIAttackOnCollide.h"

#include <algorithm>

#include "EntityLiving.h"
#include "EntityLookHelper.h"
#include "EntitySenses.h"
#include "MathHelper.h"
#include "PathEntity.h"
#include "PathNavigate.h"
#include "World.h"

EntityAIAttackOnCollide::EntityAIAttackOnCollide(EntityLiving *owner, const std::type_info &targetType, float speed, bool rememberTarget)
	: EntityAIAttackOnCollide(owner, speed, rememberTarget)
{
	classTarget = &targetType;
}

EntityAIAttackOnCollide::EntityAIAttackOnCollide(EntityLiving *owner, float speed, bool rememberTarget)
	: worldObj(owner != nullptr ? owner->worldObj : nullptr), attacker(owner), classTarget(nullptr), targetEntityId(-1),
	  attackTick(0), speedTowardsTarget(speed), longMemory(rememberTarget), pendingPath(nullptr), delayCounter(0)
{
	setMutexBits(3);
}

EntityAIAttackOnCollide::~EntityAIAttackOnCollide()
{
	delete pendingPath;
}

EntityLiving *EntityAIAttackOnCollide::resolveTarget() const
{
	if (worldObj == nullptr || targetEntityId < 0)
		return nullptr;
	return dynamic_cast<EntityLiving *>(worldObj->getEntityByID(targetEntityId));
}

bool EntityAIAttackOnCollide::matchesTargetType(EntityLiving *target) const
{
	return target != nullptr && (classTarget == nullptr || target->isAssignableTo(*classTarget));
}

bool EntityAIAttackOnCollide::shouldExecute()
{
	delete pendingPath;
	pendingPath = nullptr;
	targetEntityId = -1;
	if (attacker == nullptr || attacker->getNavigator() == nullptr)
		return false;
	EntityLiving *target = attacker->getAttackTarget();
	if (!matchesTargetType(target))
		return false;
	pendingPath = attacker->getNavigator()->getPathToEntityLiving(target);
	if (pendingPath == nullptr)
		return false;
	targetEntityId = target->entityId;
	return true;
}

bool EntityAIAttackOnCollide::continueExecuting()
{
	if (attacker == nullptr || attacker->getNavigator() == nullptr)
		return false;
	EntityLiving *target = resolveTarget();
	if (attacker->getAttackTarget() == nullptr || target == nullptr || !target->isEntityAlive())
		return false;
	if (!longMemory)
		return !attacker->getNavigator()->noPath();
	return attacker->isWithinHomeDistance(MathHelper::floor_double(target->posX), MathHelper::floor_double(target->posY), MathHelper::floor_double(target->posZ));
}

void EntityAIAttackOnCollide::startExecuting()
{
	if (attacker == nullptr || attacker->getNavigator() == nullptr)
		return;
	if (pendingPath != nullptr)
	{
		attacker->getNavigator()->setPath(pendingPath, speedTowardsTarget);
		pendingPath = nullptr;
	}
	delayCounter = 0;
}

void EntityAIAttackOnCollide::resetTask()
{
	delete pendingPath;
	pendingPath = nullptr;
	targetEntityId = -1;
	if (attacker != nullptr && attacker->getNavigator() != nullptr)
		attacker->getNavigator()->clearPathEntity();
}

void EntityAIAttackOnCollide::updateTask()
{
	EntityLiving *target = resolveTarget();
	if (attacker == nullptr || target == nullptr)
		return;
	if (attacker->getLookHelper() != nullptr)
		attacker->getLookHelper()->setLookPositionWithEntity(target, 30.0f, 30.0f);
	EntitySenses *senses = attacker->getEntitySenses();
	if ((longMemory || (senses != nullptr && senses->canSee(target))) && --delayCounter <= 0)
	{
		delayCounter = 4 + attacker->getRNG().nextInt(7);
		if (attacker->getNavigator() != nullptr)
			attacker->getNavigator()->tryMoveToEntityLiving(target, speedTowardsTarget);
	}
	attackTick = std::max(attackTick - 1, 0);
	double attackRange = (double)(attacker->width * 2.0f);
	attackRange *= attackRange;
	if (target->boundingBox != nullptr && attacker->getDistanceSq(target->posX, target->boundingBox->minY, target->posZ) <= attackRange && attackTick <= 0)
	{
		attackTick = 20;
		attacker->attackEntityAsMob(target);
	}
}
