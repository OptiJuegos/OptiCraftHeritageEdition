#include "EntityAIRestrictSun.h"

#include "EntityCreature.h"
#include "PathNavigate.h"
#include "World.h"

EntityAIRestrictSun::EntityAIRestrictSun(EntityCreature *entity) : theEntity(entity)
{
}

bool EntityAIRestrictSun::shouldExecute()
{
	return theEntity != nullptr && theEntity->worldObj != nullptr && theEntity->worldObj->isDaytime();
}

void EntityAIRestrictSun::startExecuting()
{
	if (theEntity != nullptr && theEntity->getNavigator() != nullptr)
		theEntity->getNavigator()->setAvoidSun(true);
}

void EntityAIRestrictSun::resetTask()
{
	if (theEntity != nullptr && theEntity->getNavigator() != nullptr)
		theEntity->getNavigator()->setAvoidSun(false);
}
