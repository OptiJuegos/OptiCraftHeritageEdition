#pragma once

#include "EntityAIBase.h"

class EntityLiving;
class World;

// net.minecraft.src.EntityAIOcelotAttack
class EntityAIOcelotAttack : public EntityAIBase
{
public:
	explicit EntityAIOcelotAttack(EntityLiving *entity);

	bool shouldExecute() override;
	bool continueExecuting() override;
	void resetTask() override;
	void updateTask() override;

private:
	EntityLiving *resolveTarget() const;

	World *world;
	EntityLiving *entity;
	int_t targetEntityId;
	int_t attackCooldown;
};
