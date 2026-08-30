#pragma once

#include "EntityAIBase.h"

class EntityAnimal;
class World;

// net.minecraft.src.EntityAIMate
class EntityAIMate : public EntityAIBase
{
public:
	EntityAIMate(EntityAnimal *animal, float speed);

	bool shouldExecute() override;
	bool continueExecuting() override;
	void resetTask() override;
	void updateTask() override;

private:
	EntityAnimal *resolveMate() const;
	int_t findMateEntityId() const;
	void spawnBaby();

	EntityAnimal *theAnimal;
	World *theWorld;
	int_t targetMateEntityId;
	int_t spawnBabyDelay;
	float moveSpeed;
};
