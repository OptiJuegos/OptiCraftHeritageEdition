#pragma once

#include "EntityAIBase.h"

class EntityLiving;

// net.minecraft.src.EntityAISwimming
class EntityAISwimming : public EntityAIBase
{
public:
	explicit EntityAISwimming(EntityLiving *entity);
	bool shouldExecute() override;
	void updateTask() override;

private:
	EntityLiving *theEntity;
};
