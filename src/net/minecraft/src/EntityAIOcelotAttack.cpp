#include "EntityAIOcelotAttack.h"

#include <algorithm>

#include "EntityLiving.h"
#include "EntityLookHelper.h"
#include "PathNavigate.h"
#include "World.h"

EntityAIOcelotAttack::EntityAIOcelotAttack(EntityLiving *owner)
	: world(owner != nullptr ? owner->worldObj : nullptr), entity(owner), targetEntityId(-1), attackCooldown(0)
{
	setMutexBits(3);
}

EntityLiving *EntityAIOcelotAttack::resolveTarget() const
{
	if (world == nullptr || targetEntityId < 0)
		return nullptr;
	return dynamic_cast<EntityLiving *>(world->getEntityByID(targetEntityId));
}

bool EntityAIOcelotAttack::shouldExecute()
{
	targetEntityId = -1;
	if (entity == nullptr)
		return false;
	EntityLiving *target = entity->getAttackTarget();
	if (target == nullptr)
		return false;
	targetEntityId = target->entityId;
	return true;
}

bool EntityAIOcelotAttack::continueExecuting()
{
	EntityLiving *target = resolveTarget();
	return entity != nullptr && target != nullptr && target->isEntityAlive() &&
		entity->getDistanceSqToEntity(target) <= 225.0 &&
		(entity->getNavigator() == nullptr || !entity->getNavigator()->noPath() || shouldExecute());
}

void EntityAIOcelotAttack::resetTask()
{
	targetEntityId = -1;
	if (entity != nullptr && entity->getNavigator() != nullptr)
		entity->getNavigator()->clearPathEntity();
}

void EntityAIOcelotAttack::updateTask()
{
	EntityLiving *target = resolveTarget();
	if (entity == nullptr || target == nullptr)
		return;
	if (entity->getLookHelper() != nullptr)
		entity->getLookHelper()->setLookPositionWithEntity(target, 30.0f, 30.0f);

	double reachSq = (double)(entity->width * 2.0f * entity->width * 2.0f);
	double distanceSq = entity->getDistanceSq(target->posX, target->boundingBox->minY, target->posZ);
	float speed = 0.23f;
	if (distanceSq > reachSq && distanceSq < 16.0)
		speed = 0.4f;
	else if (distanceSq < 225.0)
		speed = 0.18f;
	if (entity->getNavigator() != nullptr)
		entity->getNavigator()->tryMoveToEntityLiving(target, speed);

	attackCooldown = std::max(attackCooldown - 1, 0);
	if (distanceSq <= reachSq && attackCooldown <= 0)
	{
		attackCooldown = 20;
		entity->attackEntityAsMob(target);
	}
}
