#include "EntityAIOpenDoor.h"

#include "BlockDoor.h"
#include "EntityLiving.h"
#include "World.h"

EntityAIOpenDoor::EntityAIOpenDoor(EntityLiving *entity, bool closeDoor)
	: EntityAIDoorInteract(entity), closeDoor(closeDoor), closeDoorTemporisation(0)
{
}

bool EntityAIOpenDoor::continueExecuting()
{
	return closeDoor && closeDoorTemporisation > 0 && EntityAIDoorInteract::continueExecuting();
}

void EntityAIOpenDoor::startExecuting()
{
	closeDoorTemporisation = 20;
	if (targetDoor != nullptr)
		targetDoor->onPoweredBlockChange(theEntity->worldObj, entityPosX, entityPosY, entityPosZ, true);
}

void EntityAIOpenDoor::resetTask()
{
	if (closeDoor && targetDoor != nullptr)
		targetDoor->onPoweredBlockChange(theEntity->worldObj, entityPosX, entityPosY, entityPosZ, false);
	targetDoor = nullptr;
}

void EntityAIOpenDoor::updateTask()
{
	--closeDoorTemporisation;
	EntityAIDoorInteract::updateTask();
}
