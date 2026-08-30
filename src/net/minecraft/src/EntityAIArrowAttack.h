#pragma once

#include "EntityAIBase.h"
#include "java/Type.h"

class EntityLiving;
class World;

// net.minecraft.src.EntityAIArrowAttack
class EntityAIArrowAttack : public EntityAIBase
{
public:
	EntityAIArrowAttack(EntityLiving *host, float moveSpeed, int_t rangedAttackId, int_t maxAttackTime);

	bool shouldExecute() override;
	bool continueExecuting() override;
	void resetTask() override;
	void updateTask() override;

private:
	EntityLiving *resolveTarget() const;
	void doRangedAttack(EntityLiving *target);

	World *worldObj;
	EntityLiving *entityHost;
	int_t targetEntityId;
	int_t rangedAttackTime;
	float moveSpeed;
	int_t seeTime;
	int_t rangedAttackId;
	int_t maxRangedAttackTime;
};
