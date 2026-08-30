#pragma once

#include "EntityAITarget.h"

class EntityTameable;

// net.minecraft.src.EntityAIOwnerHurtTarget
class EntityAIOwnerHurtTarget : public EntityAITarget
{
public:
	explicit EntityAIOwnerHurtTarget(EntityTameable *tameable);

	bool shouldExecute() override;
	void startExecuting() override;

private:
	EntityLiving *resolveOwnerTarget() const;

	EntityTameable *tameable;
	int_t ownerTargetEntityId;
};
