#include "EntityAIBreakDoor.h"

#include "BlockDoor.h"
#include "EntityLiving.h"
#include "World.h"

EntityAIBreakDoor::EntityAIBreakDoor(EntityLiving *entity)
	: EntityAIDoorInteract(entity), breakingTime(0)
{
}

bool EntityAIBreakDoor::isDoorOpen() const
{
	if (theEntity == nullptr || theEntity->worldObj == nullptr || targetDoor == nullptr)
		return false;
	return BlockDoor::isOpen(targetDoor->getFullMetadata(theEntity->worldObj, entityPosX, entityPosY, entityPosZ));
}

bool EntityAIBreakDoor::shouldExecute()
{
	return EntityAIDoorInteract::shouldExecute() && !isDoorOpen();
}

void EntityAIBreakDoor::startExecuting()
{
	EntityAIDoorInteract::startExecuting();
	breakingTime = 240;
}

bool EntityAIBreakDoor::continueExecuting()
{
	if (theEntity == nullptr || targetDoor == nullptr)
		return false;
	double distanceSq = theEntity->getDistanceSq((double)entityPosX, (double)entityPosY, (double)entityPosZ);
	return breakingTime >= 0 && !isDoorOpen() && distanceSq < 4.0;
}

void EntityAIBreakDoor::updateTask()
{
	EntityAIDoorInteract::updateTask();
	if (theEntity == nullptr || theEntity->worldObj == nullptr || targetDoor == nullptr)
		return;
	if (theEntity->getRNG().nextInt(20) == 0)
		theEntity->worldObj->playAuxSFX(1010, entityPosX, entityPosY, entityPosZ, 0);
	if (--breakingTime == 0 && theEntity->worldObj->difficultySetting == 3)
	{
		theEntity->worldObj->setBlockWithNotify(entityPosX, entityPosY, entityPosZ, 0);
		theEntity->worldObj->playAuxSFX(1012, entityPosX, entityPosY, entityPosZ, 0);
		theEntity->worldObj->playAuxSFX(2001, entityPosX, entityPosY, entityPosZ, targetDoor->blockID);
	}
}
