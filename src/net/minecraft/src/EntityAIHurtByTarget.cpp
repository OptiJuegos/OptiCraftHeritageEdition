#include "EntityAIHurtByTarget.h"

#include <vector>

#include "AxisAlignedBB.h"
#include "Entity.h"
#include "EntityLiving.h"
#include "World.h"

EntityAIHurtByTarget::EntityAIHurtByTarget(EntityLiving *owner, bool notifyOthers)
	: EntityAITarget(owner, 16.0f, false), callsForHelp(notifyOthers)
{
	setMutexBits(1);
}

bool EntityAIHurtByTarget::shouldExecute()
{
	return taskOwner != nullptr && isSuitableTarget(taskOwner->getAITarget(), true);
}

void EntityAIHurtByTarget::startExecuting()
{
	if (taskOwner == nullptr)
		return;
	EntityLiving *revengeTarget = taskOwner->getAITarget();
	taskOwner->setAttackTarget(revengeTarget);
	if (callsForHelp && revengeTarget != nullptr && taskOwner->worldObj != nullptr && taskOwner->boundingBox != nullptr)
	{
		AxisAlignedBB *search = AxisAlignedBB::getBoundingBoxFromPool(taskOwner->posX, taskOwner->posY, taskOwner->posZ,
			taskOwner->posX + 1.0, taskOwner->posY + 1.0, taskOwner->posZ + 1.0)->expand((double)targetDistance, 4.0, (double)targetDistance);
		const std::vector<Entity *> &nearby = taskOwner->worldObj->getEntitiesWithinAABB(typeid(*taskOwner), search);
		for (Entity *entity : nearby)
		{
			EntityLiving *living = dynamic_cast<EntityLiving *>(entity);
			if (living != nullptr && living != taskOwner && living->getAttackTarget() == nullptr)
				living->setAttackTarget(revengeTarget);
		}
	}
	EntityAITarget::startExecuting();
}
