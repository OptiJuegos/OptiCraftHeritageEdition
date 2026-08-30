#pragma once

#include "EntityAIBase.h"

class EntityTameable;
class EntityLiving;
class PathNavigate;
class World;

// net.minecraft.src.EntityAIFollowOwner
class EntityAIFollowOwner : public EntityAIBase
{
public:
	EntityAIFollowOwner(EntityTameable *pet, float speed, float minDistance, float maxDistance);

	bool shouldExecute() override;
	bool continueExecuting() override;
	void startExecuting() override;
	void resetTask() override;
	void updateTask() override;

private:
	EntityLiving *resolveOwner() const;

	EntityTameable *pet;
	World *world;
	float speed;
	PathNavigate *navigator;
	int_t ownerEntityId;
	int_t pathRecalcTimer;
	float minDistance;
	float maxDistance;
	bool oldAvoidsWater;
};
