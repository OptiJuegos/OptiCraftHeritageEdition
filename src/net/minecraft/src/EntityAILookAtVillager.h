#pragma once

#include "EntityAIBase.h"

class EntityIronGolem;
class EntityVillager;

// net.minecraft.src.EntityAILookAtVillager
class EntityAILookAtVillager : public EntityAIBase
{
public:
	explicit EntityAILookAtVillager(EntityIronGolem *golem);

	bool shouldExecute() override;
	bool continueExecuting() override;
	void startExecuting() override;
	void resetTask() override;
	void updateTask() override;

private:
	EntityVillager *resolveVillager() const;

	EntityIronGolem *theGolem;
	int_t villagerEntityId;
	int_t lookTime;
};
