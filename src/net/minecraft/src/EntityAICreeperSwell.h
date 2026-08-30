#pragma once

#include "EntityAIBase.h"

class EntityCreeper;
class EntityLiving;

// net.minecraft.src.EntityAICreeperSwell
class EntityAICreeperSwell : public EntityAIBase
{
public:
	explicit EntityAICreeperSwell(EntityCreeper *creeper);

	bool shouldExecute() override;
	void startExecuting() override;
	void resetTask() override;
	void updateTask() override;

private:
	EntityLiving *resolveTarget() const;

	EntityCreeper *creeper;
	int_t targetEntityId;
};
