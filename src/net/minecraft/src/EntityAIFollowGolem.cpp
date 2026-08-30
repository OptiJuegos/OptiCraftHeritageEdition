#include "EntityAIFollowGolem.h"

#include "AxisAlignedBB.h"
#include "Entity.h"
#include "EntityIronGolem.h"
#include "EntityLookHelper.h"
#include "EntityVillager.h"
#include "PathNavigate.h"
#include "World.h"

EntityAIFollowGolem::EntityAIFollowGolem(EntityVillager *villager)
	: theVillager(villager), golemEntityId(-1), roseTickTarget(0), movingToGolem(false)
{
	setMutexBits(3);
}

EntityIronGolem *EntityAIFollowGolem::resolveGolem() const
{
	if (theVillager == nullptr || theVillager->worldObj == nullptr || golemEntityId < 0)
		return nullptr;
	return dynamic_cast<EntityIronGolem *>(theVillager->worldObj->getEntityByID(golemEntityId));
}

bool EntityAIFollowGolem::shouldExecute()
{
	golemEntityId = -1;
	if (theVillager == nullptr || theVillager->worldObj == nullptr || theVillager->getGrowingAge() >= 0 || !theVillager->worldObj->isDaytime())
		return false;
	const std::vector<Entity *> &entities = theVillager->worldObj->getEntitiesWithinAABB(typeid(EntityIronGolem),
		theVillager->boundingBox->expand(6.0, 2.0, 6.0));
	for (Entity *entity : entities)
	{
		EntityIronGolem *golem = dynamic_cast<EntityIronGolem *>(entity);
		if (golem != nullptr && !golem->isDead && golem->getHoldRoseTick() > 0)
		{
			golemEntityId = golem->entityId;
			break;
		}
	}
	return golemEntityId >= 0;
}

bool EntityAIFollowGolem::continueExecuting()
{
	EntityIronGolem *golem = resolveGolem();
	return golem != nullptr && !golem->isDead && golem->getHoldRoseTick() > 0;
}

void EntityAIFollowGolem::startExecuting()
{
	if (theVillager == nullptr)
		return;
	roseTickTarget = theVillager->getRNG().nextInt(320);
	movingToGolem = false;
	EntityIronGolem *golem = resolveGolem();
	if (golem != nullptr && golem->getNavigator() != nullptr)
		golem->getNavigator()->clearPathEntity();
}

void EntityAIFollowGolem::resetTask()
{
	golemEntityId = -1;
	movingToGolem = false;
	if (theVillager != nullptr && theVillager->getNavigator() != nullptr)
		theVillager->getNavigator()->clearPathEntity();
}

void EntityAIFollowGolem::updateTask()
{
	EntityIronGolem *golem = resolveGolem();
	if (theVillager == nullptr || golem == nullptr)
		return;
	theVillager->getLookHelper()->setLookPositionWithEntity(golem, 30.0f, 30.0f);
	if (golem->getHoldRoseTick() == roseTickTarget)
	{
		theVillager->getNavigator()->tryMoveToEntityLiving(golem, 0.15f);
		movingToGolem = true;
	}
	if (movingToGolem && theVillager->getDistanceSqToEntity(golem) < 4.0)
	{
		golem->setHoldingRose(false);
		theVillager->getNavigator()->clearPathEntity();
	}
}
