#pragma once

#include <typeinfo>

#include "EntityAIBase.h"

class Entity;
class EntityLiving;

// net.minecraft.src.EntityAIWatchClosest
class EntityAIWatchClosest : public EntityAIBase
{
public:
	EntityAIWatchClosest(EntityLiving *entity, const std::type_info &targetType, float maxDistance);
	EntityAIWatchClosest(EntityLiving *entity, const std::type_info &targetType, float maxDistance, float chance);

	bool shouldExecute() override;
	bool continueExecuting() override;
	void startExecuting() override;
	void resetTask() override;
	void updateTask() override;

protected:
	bool matchesTargetType(Entity *entity) const;

	EntityLiving *watcher;
	Entity *resolveClosestEntity() const;

	int_t closestEntityId;
	float maxDistance;
	int_t lookTime;
	float chance;
	const std::type_info *targetType;
};
