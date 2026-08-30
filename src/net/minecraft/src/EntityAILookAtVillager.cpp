#include "EntityAILookAtVillager.h"

#include "AxisAlignedBB.h"
#include "Entity.h"
#include "EntityIronGolem.h"
#include "EntityLookHelper.h"
#include "EntityVillager.h"
#include "World.h"

EntityAILookAtVillager::EntityAILookAtVillager(EntityIronGolem *golem)
	: theGolem(golem), villagerEntityId(-1), lookTime(0)
{
	setMutexBits(3);
}

EntityVillager *EntityAILookAtVillager::resolveVillager() const
{
	if (theGolem == nullptr || theGolem->worldObj == nullptr || villagerEntityId < 0)
		return nullptr;
	return dynamic_cast<EntityVillager *>(theGolem->worldObj->getEntityByID(villagerEntityId));
}

bool EntityAILookAtVillager::shouldExecute()
{
	villagerEntityId = -1;
	if (theGolem == nullptr || theGolem->worldObj == nullptr || !theGolem->worldObj->isDaytime())
		return false;
	if (theGolem->getRNG().nextInt(8000) != 0)
		return false;
	Entity *entity = theGolem->worldObj->findNearestEntityWithinAABB(typeid(EntityVillager),
		theGolem->boundingBox->expand(6.0, 2.0, 6.0), theGolem);
	EntityVillager *villager = dynamic_cast<EntityVillager *>(entity);
	if (villager == nullptr)
		return false;
	villagerEntityId = villager->entityId;
	return true;
}

bool EntityAILookAtVillager::continueExecuting()
{
	EntityVillager *villager = resolveVillager();
	return villager != nullptr && !villager->isDead && lookTime > 0;
}

void EntityAILookAtVillager::startExecuting()
{
	lookTime = 400;
	if (theGolem != nullptr)
		theGolem->setHoldingRose(true);
}

void EntityAILookAtVillager::resetTask()
{
	if (theGolem != nullptr)
		theGolem->setHoldingRose(false);
	villagerEntityId = -1;
	lookTime = 0;
}

void EntityAILookAtVillager::updateTask()
{
	EntityVillager *villager = resolveVillager();
	if (theGolem == nullptr || villager == nullptr)
		return;
	theGolem->getLookHelper()->setLookPositionWithEntity(villager, 30.0f, 30.0f);
	--lookTime;
}
