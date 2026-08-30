#pragma once

#include "EntityAIBase.h"
#include "java/Type.h"

class EntityLiving;

// net.minecraft.src.EntityAILeapAtTarget
class EntityAILeapAtTarget : public EntityAIBase
{
public:
	EntityAILeapAtTarget(EntityLiving *entity, float leapMotionY);

	bool shouldExecute() override;
	bool continueExecuting() override;
	void startExecuting() override;
	void resetTask() override;

private:
	EntityLiving *resolveTarget() const;

	EntityLiving *leaper;
	int_t targetEntityId;
	float leapMotionY;
};
