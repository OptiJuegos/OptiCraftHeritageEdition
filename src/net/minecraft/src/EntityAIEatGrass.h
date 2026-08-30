#pragma once

#include "EntityAIBase.h"
#include "java/Type.h"

class EntityLiving;
class World;

// net.minecraft.src.EntityAIEatGrass
class EntityAIEatGrass : public EntityAIBase
{
public:
	explicit EntityAIEatGrass(EntityLiving *entity);

	bool shouldExecute() override;
	bool continueExecuting() override;
	void startExecuting() override;
	void resetTask() override;
	void updateTask() override;
	int_t getEatGrassTick() const;
	int_t func_48396_h() const;

private:
	EntityLiving *theEntity;
	World *theWorld;
	int_t eatGrassTick;
};
