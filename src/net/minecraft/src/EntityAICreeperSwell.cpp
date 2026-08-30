#include "EntityAICreeperSwell.h"

#include "EntityCreeper.h"
#include "EntityLiving.h"
#include "EntitySenses.h"
#include "PathNavigate.h"
#include "World.h"

EntityAICreeperSwell::EntityAICreeperSwell(EntityCreeper *entity)
	: creeper(entity), targetEntityId(-1)
{
	setMutexBits(1);
}

EntityLiving *EntityAICreeperSwell::resolveTarget() const
{
	if (creeper == nullptr || creeper->worldObj == nullptr || targetEntityId < 0)
		return nullptr;
	return dynamic_cast<EntityLiving *>(creeper->worldObj->getEntityByID(targetEntityId));
}

bool EntityAICreeperSwell::shouldExecute()
{
	if (creeper == nullptr)
		return false;
	EntityLiving *target = creeper->getAttackTarget();
	return creeper->getCreeperState() > 0 || (target != nullptr && creeper->getDistanceSqToEntity(target) < 9.0);
}

void EntityAICreeperSwell::startExecuting()
{
	if (creeper == nullptr)
		return;
	if (creeper->getNavigator() != nullptr)
		creeper->getNavigator()->clearPathEntity();
	EntityLiving *target = creeper->getAttackTarget();
	targetEntityId = target != nullptr ? target->entityId : -1;
}

void EntityAICreeperSwell::resetTask()
{
	targetEntityId = -1;
}

void EntityAICreeperSwell::updateTask()
{
	if (creeper == nullptr)
		return;
	EntityLiving *target = resolveTarget();
	if (target == nullptr || !target->isEntityAlive() || creeper->getDistanceSqToEntity(target) > 49.0)
	{
		creeper->setCreeperState(-1);
		return;
	}
	EntitySenses *senses = creeper->getEntitySenses();
	creeper->setCreeperState(senses != nullptr && senses->canSee(target) ? 1 : -1);
}
