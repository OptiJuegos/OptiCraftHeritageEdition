#include "EntityAINearestAttackableTarget.h"

#include <algorithm>
#include <vector>

#include "Entity.h"
#include "EntityLiving.h"
#include "EntityPlayer.h"
#include "World.h"

EntityAINearestAttackableTarget::EntityAINearestAttackableTarget(EntityLiving *owner, const std::type_info &targetType,
	float distance, int_t chance, bool checkSight)
	: EntityAINearestAttackableTarget(owner, targetType, distance, chance, checkSight, false)
{
}

EntityAINearestAttackableTarget::EntityAINearestAttackableTarget(EntityLiving *owner, const std::type_info &targetType,
	float distance, int_t chance, bool checkSight, bool nearbyOnly)
	: EntityAITarget(owner, distance, checkSight, nearbyOnly), targetClass(&targetType), targetChance(chance), targetEntityId(-1)
{
	setMutexBits(1);
}

EntityLiving *EntityAINearestAttackableTarget::resolveTarget() const
{
	if (taskOwner == nullptr || taskOwner->worldObj == nullptr || targetEntityId < 0)
		return nullptr;
	return dynamic_cast<EntityLiving *>(taskOwner->worldObj->getEntityByID(targetEntityId));
}

bool EntityAINearestAttackableTarget::shouldExecute()
{
	targetEntityId = -1;
	if (taskOwner == nullptr || taskOwner->worldObj == nullptr || targetClass == nullptr)
		return false;
	if (targetChance > 0 && taskOwner->getRNG().nextInt(targetChance) != 0)
		return false;
	if (*targetClass == typeid(EntityPlayer))
	{
		EntityPlayer *player = taskOwner->worldObj->getClosestVulnerablePlayerToEntity(taskOwner, (double)targetDistance);
		if (isSuitableTarget(player, false))
		{
			targetEntityId = player->entityId;
			return true;
		}
		return false;
	}

	const std::vector<Entity *> &source = taskOwner->worldObj->getEntitiesWithinAABB(*targetClass,
		taskOwner->boundingBox->expand((double)targetDistance, 4.0, (double)targetDistance));
	std::vector<EntityLiving *> candidates;
	candidates.reserve(source.size());
	for (Entity *entity : source)
	{
		EntityLiving *living = dynamic_cast<EntityLiving *>(entity);
		if (living != nullptr)
			candidates.push_back(living);
	}
	std::stable_sort(candidates.begin(), candidates.end(), [this](EntityLiving *left, EntityLiving *right)
	{
		return taskOwner->getDistanceSqToEntity(left) < taskOwner->getDistanceSqToEntity(right);
	});
	for (EntityLiving *candidate : candidates)
	{
		if (isSuitableTarget(candidate, false))
		{
			targetEntityId = candidate->entityId;
			return true;
		}
	}
	return false;
}

void EntityAINearestAttackableTarget::startExecuting()
{
	if (taskOwner != nullptr)
		taskOwner->setAttackTarget(resolveTarget());
	EntityAITarget::startExecuting();
}
