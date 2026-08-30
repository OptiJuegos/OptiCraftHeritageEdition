#pragma once

#include "EntityAIBase.h"

class BlockDoor;
class EntityLiving;

// net.minecraft.src.EntityAIDoorInteract
class EntityAIDoorInteract : public EntityAIBase
{
public:
	explicit EntityAIDoorInteract(EntityLiving *entity);
	bool shouldExecute() override;
	bool continueExecuting() override;
	void startExecuting() override;
	void updateTask() override;

protected:
	BlockDoor *getDoorBlock(int_t x, int_t y, int_t z) const;

	EntityLiving *theEntity;
	int_t entityPosX;
	int_t entityPosY;
	int_t entityPosZ;
	BlockDoor *targetDoor;
	bool hasStoppedDoorInteraction;
	float entityPositionX;
	float entityPositionZ;
};
