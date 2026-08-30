#pragma once

#include "EntityAITarget.h"

class EntityIronGolem;
class EntityLiving;

// net.minecraft.src.EntityAIDefendVillage
class EntityAIDefendVillage : public EntityAITarget
{
public:
	explicit EntityAIDefendVillage(EntityIronGolem *golem);

	bool shouldExecute() override;
	void startExecuting() override;

private:
	EntityLiving *resolveTarget() const;

	EntityIronGolem *ironGolem;
	int_t villageAggressorEntityId;
};
