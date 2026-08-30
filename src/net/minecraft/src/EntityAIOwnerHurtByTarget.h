#pragma once

#include "EntityAITarget.h"

class EntityTameable;

// net.minecraft.src.EntityAIOwnerHurtByTarget
class EntityAIOwnerHurtByTarget : public EntityAITarget
{
public:
	explicit EntityAIOwnerHurtByTarget(EntityTameable *tameable);

	bool shouldExecute() override;
	void startExecuting() override;

private:
	EntityLiving *resolveOwnerTarget() const;

	EntityTameable *tameable;
	int_t ownerTargetEntityId;
};
