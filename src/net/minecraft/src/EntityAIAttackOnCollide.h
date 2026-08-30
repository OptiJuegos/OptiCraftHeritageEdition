#pragma once

#include <typeinfo>

#include "EntityAIBase.h"
#include "java/Type.h"

class EntityLiving;
class PathEntity;
class World;

// net.minecraft.src.EntityAIAttackOnCollide
class EntityAIAttackOnCollide : public EntityAIBase
{
public:
	EntityAIAttackOnCollide(EntityLiving *attacker, const std::type_info &targetType, float speed, bool longMemory);
	EntityAIAttackOnCollide(EntityLiving *attacker, float speed, bool longMemory);
	~EntityAIAttackOnCollide() override;

	bool shouldExecute() override;
	bool continueExecuting() override;
	void startExecuting() override;
	void resetTask() override;
	void updateTask() override;

private:
	EntityLiving *resolveTarget() const;
	bool matchesTargetType(EntityLiving *target) const;

	World *worldObj;
	EntityLiving *attacker;
	const std::type_info *classTarget;
	int_t targetEntityId;
	int_t attackTick;
	float speedTowardsTarget;
	bool longMemory;
	PathEntity *pendingPath;
	int_t delayCounter;
};
