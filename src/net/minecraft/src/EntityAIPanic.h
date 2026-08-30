#pragma once

#include "EntityAIBase.h"

class EntityCreature;

// net.minecraft.src.EntityAIPanic
class EntityAIPanic : public EntityAIBase
{
public:
	EntityAIPanic(EntityCreature *entity, float speed);

	bool shouldExecute() override;
	bool continueExecuting() override;
	void startExecuting() override;

private:
	EntityCreature *theEntity;
	float speed;
	double targetX;
	double targetY;
	double targetZ;
};
