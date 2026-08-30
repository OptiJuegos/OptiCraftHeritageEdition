#pragma once

#include "EntityAIBase.h"

class EntityCreature;

// net.minecraft.src.EntityAIMoveIndoors
class EntityAIMoveIndoors : public EntityAIBase
{
public:
	explicit EntityAIMoveIndoors(EntityCreature *entity);
	bool shouldExecute() override;
	bool continueExecuting() override;
	void startExecuting() override;
	void resetTask() override;

private:
	EntityCreature *entityObj;
	int_t doorInsideX;
	int_t doorInsideY;
	int_t doorInsideZ;
	int_t insidePosX;
	int_t insidePosZ;
};
