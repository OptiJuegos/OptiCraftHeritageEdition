#pragma once

#include "EntityAIBase.h"

class EntityIronGolem;
class EntityVillager;

// net.minecraft.src.EntityAIFollowGolem
class EntityAIFollowGolem : public EntityAIBase
{
public:
	explicit EntityAIFollowGolem(EntityVillager *villager);

	bool shouldExecute() override;
	bool continueExecuting() override;
	void startExecuting() override;
	void resetTask() override;
	void updateTask() override;

private:
	EntityIronGolem *resolveGolem() const;

	EntityVillager *theVillager;
	int_t golemEntityId;
	int_t roseTickTarget;
	bool movingToGolem;
};
