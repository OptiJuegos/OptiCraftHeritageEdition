#pragma once

#include "EntityAIBase.h"

class EntityLiving;
class EntityVillager;
class Village;
class World;

// net.minecraft.src.EntityAIVillagerMate
class EntityAIVillagerMate : public EntityAIBase
{
public:
	explicit EntityAIVillagerMate(EntityVillager *villager);

	bool shouldExecute() override;
	bool continueExecuting() override;
	void startExecuting() override;
	void resetTask() override;
	void updateTask() override;

private:
	EntityVillager *resolveMate() const;
	Village *resolveVillage() const;
	bool checkSufficientDoorsPresentForNewVillager() const;
	void giveBirth(EntityVillager *mate);
	void spawnHeartParticles(EntityLiving *entity);

	EntityVillager *villagerObj;
	World *worldObj;
	int_t mateEntityId;
	int_t matingTimeout;
};
