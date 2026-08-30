#include "EntityAIWatchClosest2.h"

EntityAIWatchClosest2::EntityAIWatchClosest2(EntityLiving *entity, const std::type_info &targetType, float maxDistance)
	: EntityAIWatchClosest(entity, targetType, maxDistance)
{
	setMutexBits(3);
}

EntityAIWatchClosest2::EntityAIWatchClosest2(EntityLiving *entity, const std::type_info &targetType, float maxDistance, float chance)
	: EntityAIWatchClosest(entity, targetType, maxDistance, chance)
{
	setMutexBits(3);
}
