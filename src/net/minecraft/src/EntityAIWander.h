#pragma once

#include "EntityAIBase.h"

class EntityCreature;

// net.minecraft.src.EntityAIWander
class EntityAIWander : public EntityAIBase
{
public:
	EntityAIWander(EntityCreature *entity, float speed);
	bool shouldExecute() override;
	bool continueExecuting() override;
	void startExecuting() override;

private:
	EntityCreature *entity;
	double moveX;
	double moveY;
	double moveZ;
	float speed;
};
