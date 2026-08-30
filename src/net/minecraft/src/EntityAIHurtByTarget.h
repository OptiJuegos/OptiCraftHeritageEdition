#pragma once

#include "EntityAITarget.h"

// net.minecraft.src.EntityAIHurtByTarget
class EntityAIHurtByTarget : public EntityAITarget
{
public:
	EntityAIHurtByTarget(EntityLiving *owner, bool callsForHelp);

	bool shouldExecute() override;
	void startExecuting() override;

private:
	bool callsForHelp;
};
