#pragma once

#include "EntityAIDoorInteract.h"

// net.minecraft.src.EntityAIOpenDoor
class EntityAIOpenDoor : public EntityAIDoorInteract
{
public:
	EntityAIOpenDoor(EntityLiving *entity, bool closeDoor);
	bool continueExecuting() override;
	void startExecuting() override;
	void resetTask() override;
	void updateTask() override;

private:
	bool closeDoor;
	int_t closeDoorTemporisation;
};
