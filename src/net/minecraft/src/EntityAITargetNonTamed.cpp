#include "EntityAITargetNonTamed.h"

#include "EntityTameable.h"

EntityAITargetNonTamed::EntityAITargetNonTamed(EntityTameable *entity, const std::type_info &targetType,
	float distance, int_t chance, bool checkSight)
	: EntityAINearestAttackableTarget(entity, targetType, distance, chance, checkSight), tameable(entity)
{
}

bool EntityAITargetNonTamed::shouldExecute()
{
	return tameable != nullptr && !tameable->isTamed() && EntityAINearestAttackableTarget::shouldExecute();
}
