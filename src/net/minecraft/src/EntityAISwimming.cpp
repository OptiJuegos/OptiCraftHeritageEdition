#include "EntityAISwimming.h"

#include "EntityJumpHelper.h"
#include "EntityLiving.h"
#include "PathNavigate.h"

EntityAISwimming::EntityAISwimming(EntityLiving *entity) : theEntity(entity)
{
	setMutexBits(4);
	if (theEntity != nullptr && theEntity->getNavigator() != nullptr)
		theEntity->getNavigator()->setCanSwim(true);
}

bool EntityAISwimming::shouldExecute()
{
	return theEntity != nullptr && (theEntity->isInWater() || theEntity->handleLavaMovement());
}

void EntityAISwimming::updateTask()
{
	if (theEntity != nullptr && theEntity->getRNG().nextFloat() < 0.8f && theEntity->getJumpHelper() != nullptr)
		theEntity->getJumpHelper()->setJumping();
}
