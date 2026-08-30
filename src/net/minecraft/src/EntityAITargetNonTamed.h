#pragma once

#include "EntityAINearestAttackableTarget.h"

class EntityTameable;

// net.minecraft.src.EntityAITargetNonTamed
class EntityAITargetNonTamed : public EntityAINearestAttackableTarget
{
public:
	EntityAITargetNonTamed(EntityTameable *tameable, const std::type_info &targetType, float targetDistance,
		int_t targetChance, bool shouldCheckSight);

	bool shouldExecute() override;

private:
	EntityTameable *tameable;
};
