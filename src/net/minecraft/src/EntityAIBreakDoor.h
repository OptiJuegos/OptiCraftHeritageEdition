#pragma once

#include "EntityAIDoorInteract.h"
#include "java/Type.h"

// net.minecraft.src.EntityAIBreakDoor
class EntityAIBreakDoor : public EntityAIDoorInteract
{
public:
	explicit EntityAIBreakDoor(EntityLiving *entity);

	bool shouldExecute() override;
	bool continueExecuting() override;
	void startExecuting() override;
	void updateTask() override;

private:
	bool isDoorOpen() const;

	int_t breakingTime;
};
