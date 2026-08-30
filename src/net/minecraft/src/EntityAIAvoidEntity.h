#pragma once

#include <typeinfo>

#include "EntityAIBase.h"

class Entity;
class EntityCreature;
class PathEntity;
class PathNavigate;

// net.minecraft.src.EntityAIAvoidEntity
class EntityAIAvoidEntity : public EntityAIBase
{
public:
	EntityAIAvoidEntity(EntityCreature *entity, const std::type_info &targetType, float avoidDistance, float farSpeed, float nearSpeed);
	~EntityAIAvoidEntity() override;

	bool shouldExecute() override;
	bool continueExecuting() override;
	void startExecuting() override;
	void resetTask() override;
	void updateTask() override;

private:
	Entity *resolveTarget() const;

	EntityCreature *theEntity;
	const std::type_info *targetType;
	float farSpeed;
	float nearSpeed;
	int_t targetEntityId;
	float avoidDistance;
	PathEntity *path;
	PathNavigate *navigator;
};
