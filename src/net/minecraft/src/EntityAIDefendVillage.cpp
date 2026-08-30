#include "EntityAIDefendVillage.h"

#include "EntityIronGolem.h"
#include "EntityLiving.h"
#include "Village.h"
#include "World.h"

EntityAIDefendVillage::EntityAIDefendVillage(EntityIronGolem *golem)
	: EntityAITarget(golem, 16.0f, false, true), ironGolem(golem), villageAggressorEntityId(-1)
{
	setMutexBits(1);
}

EntityLiving *EntityAIDefendVillage::resolveTarget() const
{
	if (ironGolem == nullptr || ironGolem->worldObj == nullptr || villageAggressorEntityId < 0)
		return nullptr;
	return dynamic_cast<EntityLiving *>(ironGolem->worldObj->getEntityByID(villageAggressorEntityId));
}

bool EntityAIDefendVillage::shouldExecute()
{
	villageAggressorEntityId = -1;
	if (ironGolem == nullptr)
		return false;
	Village *village = ironGolem->getVillage();
	if (village == nullptr)
		return false;
	EntityLiving *target = village->findNearestVillageAggressor(ironGolem);
	if (!isSuitableTarget(target, false))
		return false;
	villageAggressorEntityId = target->entityId;
	return true;
}

void EntityAIDefendVillage::startExecuting()
{
	EntityLiving *target = resolveTarget();
	if (ironGolem != nullptr)
		ironGolem->setAttackTarget(target);
	EntityAITarget::startExecuting();
}
