#pragma once

#include "EntityAIBase.h"
#include "java/Type.h"

class EntityCreature;
class EntityLiving;

// net.minecraft.src.EntityAIMoveTowardsTarget
class EntityAIMoveTowardsTarget : public EntityAIBase
{
public:
	EntityAIMoveTowardsTarget(EntityCreature *entity, float speed, float maxTargetDistance);

	bool shouldExecute() override;
	bool continueExecuting() override;
	void startExecuting() override;
	void resetTask() override;

private:
	EntityLiving *resolveTarget() const;

	EntityCreature *theEntity;
	int_t targetEntityId;
	double movePosX;
	double movePosY;
	double movePosZ;
	float speed;
	float maxTargetDistance;
};
