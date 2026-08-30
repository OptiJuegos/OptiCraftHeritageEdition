#pragma once

#include "EntityAIBase.h"

class EntityOcelot;
class World;

// net.minecraft.src.EntityAIOcelotSit
class EntityAIOcelotSit : public EntityAIBase
{
public:
	EntityAIOcelotSit(EntityOcelot *ocelot, float speed);

	bool shouldExecute() override;
	bool continueExecuting() override;
	void startExecuting() override;
	void resetTask() override;
	void updateTask() override;

private:
	bool findSittingSpot();
	bool isSittableBlock(World *world, int_t x, int_t y, int_t z) const;

	EntityOcelot *ocelot;
	float speed;
	int_t sitTicks;
	int_t pathFailures;
	int_t maxSitTicks;
	int_t targetX;
	int_t targetY;
	int_t targetZ;
};
