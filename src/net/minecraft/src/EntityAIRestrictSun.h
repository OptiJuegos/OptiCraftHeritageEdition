#pragma once

#include "EntityAIBase.h"

class EntityCreature;

// net.minecraft.src.EntityAIRestrictSun
class EntityAIRestrictSun : public EntityAIBase
{
public:
	explicit EntityAIRestrictSun(EntityCreature *entity);

	bool shouldExecute() override;
	void startExecuting() override;
	void resetTask() override;

private:
	EntityCreature *theEntity;
};
