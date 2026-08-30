#pragma once

#include "EntityAIBase.h"

class EntityWolf;
class EntityPlayer;

// net.minecraft.src.EntityAIBeg
class EntityAIBeg : public EntityAIBase
{
public:
	EntityAIBeg(EntityWolf *wolf, float maxDistance);

	bool shouldExecute() override;
	bool continueExecuting() override;
	void startExecuting() override;
	void resetTask() override;
	void updateTask() override;

private:
	EntityPlayer *resolvePlayer() const;
	bool playerHasInterestingItem(EntityPlayer *player) const;

	EntityWolf *wolf;
	int_t playerEntityId;
	float maxDistance;
	int_t lookTime;
};
