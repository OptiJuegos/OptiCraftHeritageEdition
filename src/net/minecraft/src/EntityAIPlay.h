#pragma once

#include "EntityAIBase.h"

class EntityVillager;

// net.minecraft.src.EntityAIPlay
class EntityAIPlay : public EntityAIBase
{
public:
	EntityAIPlay(EntityVillager *villager, float speed);

	bool shouldExecute() override;
	bool continueExecuting() override;
	void startExecuting() override;
	void resetTask() override;
	void updateTask() override;

private:
	EntityVillager *resolveTarget() const;

	EntityVillager *villagerObj;
	int_t targetEntityId;
	float speed;
	int_t playTime;
};
