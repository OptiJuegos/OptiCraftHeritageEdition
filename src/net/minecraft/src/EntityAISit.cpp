#include "EntityAISit.h"

#include "EntityLiving.h"
#include "EntityTameable.h"
#include "PathNavigate.h"

EntityAISit::EntityAISit(EntityTameable *entity)
	: theEntity(entity), sittingRequested(false)
{
	setMutexBits(5);
}

bool EntityAISit::shouldExecute()
{
	if (theEntity == nullptr || !theEntity->isTamed() || theEntity->isInWater() || !theEntity->onGround)
		return false;
	EntityLiving *owner = theEntity->getOwner();
	if (owner == nullptr)
		return true;
	if (theEntity->getDistanceSqToEntity(owner) < 144.0 && owner->getAITarget() != nullptr)
		return false;
	return sittingRequested;
}

void EntityAISit::startExecuting()
{
	if (theEntity == nullptr)
		return;
	if (theEntity->getNavigator() != nullptr)
		theEntity->getNavigator()->clearPathEntity();
	theEntity->setSitting(true);
}

void EntityAISit::resetTask()
{
	if (theEntity != nullptr)
		theEntity->setSitting(false);
}

void EntityAISit::setSitting(bool value)
{
	sittingRequested = value;
}

void EntityAISit::func_48407_a(bool value)
{
	setSitting(value);
}
