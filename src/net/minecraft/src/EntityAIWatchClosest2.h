#pragma once

#include "EntityAIWatchClosest.h"

// net.minecraft.src.EntityAIWatchClosest2
class EntityAIWatchClosest2 : public EntityAIWatchClosest
{
public:
	EntityAIWatchClosest2(EntityLiving *entity, const std::type_info &targetType, float maxDistance);
	EntityAIWatchClosest2(EntityLiving *entity, const std::type_info &targetType, float maxDistance, float chance);
};
