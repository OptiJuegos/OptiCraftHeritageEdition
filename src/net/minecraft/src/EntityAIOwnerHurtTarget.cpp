#include "EntityAIOwnerHurtTarget.h"

#include "EntityLiving.h"
#include "EntityTameable.h"
#include "World.h"

EntityAIOwnerHurtTarget::EntityAIOwnerHurtTarget(EntityTameable *entity)
	: EntityAITarget(entity, 32.0f, false), tameable(entity), ownerTargetEntityId(-1)
{
	setMutexBits(1);
}

EntityLiving *EntityAIOwnerHurtTarget::resolveOwnerTarget() const
{
	if (tameable == nullptr || tameable->worldObj == nullptr || ownerTargetEntityId < 0)
		return nullptr;
	return dynamic_cast<EntityLiving *>(tameable->worldObj->getEntityByID(ownerTargetEntityId));
}

bool EntityAIOwnerHurtTarget::shouldExecute()
{
	ownerTargetEntityId = -1;
	if (tameable == nullptr || !tameable->isTamed())
		return false;
	EntityLiving *owner = tameable->getOwner();
	EntityLiving *target = owner != nullptr ? owner->getLastAttackingEntity() : nullptr;
	if (!isSuitableTarget(target, false))
		return false;
	ownerTargetEntityId = target->entityId;
	return true;
}

void EntityAIOwnerHurtTarget::startExecuting()
{
	if (taskOwner != nullptr)
		taskOwner->setAttackTarget(resolveOwnerTarget());
	EntityAITarget::startExecuting();
}
