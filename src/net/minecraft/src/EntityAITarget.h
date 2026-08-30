#pragma once

#include "EntityAIBase.h"
#include "java/Type.h"

class EntityLiving;

// net.minecraft.src.EntityAITarget
class EntityAITarget : public EntityAIBase
{
public:
	EntityAITarget(EntityLiving *owner, float targetDistance, bool shouldCheckSight);
	EntityAITarget(EntityLiving *owner, float targetDistance, bool shouldCheckSight, bool nearbyOnly);

	bool continueExecuting() override;
	void startExecuting() override;
	void resetTask() override;

protected:
	bool isSuitableTarget(EntityLiving *target, bool includeInvulnerablePlayers);
	bool func_48376_a(EntityLiving *target, bool includeInvulnerablePlayers);

	EntityLiving *taskOwner;
	float targetDistance;
	bool shouldCheckSight;

private:
	bool canEasilyReach(EntityLiving *target);

	bool nearbyOnly;
	int_t targetSearchStatus;
	int_t targetSearchDelay;
	int_t unseenTicks;
};
