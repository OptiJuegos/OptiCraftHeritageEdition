#pragma once

#include "EntityAIBase.h"

class EntityAnimal;

// net.minecraft.src.EntityAIFollowParent
class EntityAIFollowParent : public EntityAIBase
{
public:
	EntityAIFollowParent(EntityAnimal *child, float speed);

	bool shouldExecute() override;
	bool continueExecuting() override;
	void startExecuting() override;
	void resetTask() override;
	void updateTask() override;

private:
	EntityAnimal *resolveParent() const;

	EntityAnimal *childAnimal;
	int_t parentEntityId;
	float moveSpeed;
	int_t delayCounter;
};
