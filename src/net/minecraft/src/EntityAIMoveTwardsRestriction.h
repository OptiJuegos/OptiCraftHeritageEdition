#pragma once

#include "EntityAIBase.h"

class EntityCreature;

// net.minecraft.src.EntityAIMoveTwardsRestriction
class EntityAIMoveTwardsRestriction : public EntityAIBase
{
public:
	EntityAIMoveTwardsRestriction(EntityCreature *entity, float speed);
	bool shouldExecute() override;
	bool continueExecuting() override;
	void startExecuting() override;

private:
	EntityCreature *theEntity;
	double movePosX;
	double movePosY;
	double movePosZ;
	float speed;
};
