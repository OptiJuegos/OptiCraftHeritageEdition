#pragma once

#include "EntityAIBase.h"
#include "platform/PlatformTuning.h"

class EntityCreature;
class EntityPlayer;

// net.minecraft.src.EntityAITempt
class EntityAITempt : public EntityAIBase
{
public:
	EntityAITempt(EntityCreature *entity, float speed, int_t itemId, bool scaredByPlayerMovement);

	bool shouldExecute() override;
	bool continueExecuting() override;
	void startExecuting() override;
	void resetTask() override;
	void updateTask() override;
	bool isRunning() const;
	bool func_48270_h() const;

private:
#if PLATFORM_FLOAT_ENTITY_AI_MATH
	using tempt_math_t = float;
#else
	using tempt_math_t = double;
#endif

	EntityPlayer *resolvePlayer() const;
	bool playerHoldsTemptItem(EntityPlayer *player) const;

	EntityCreature *temptedEntity;
	float moveSpeed;
	tempt_math_t targetX;
	tempt_math_t targetY;
	tempt_math_t targetZ;
	tempt_math_t targetPitch;
	tempt_math_t targetYaw;
	int_t temptingPlayerEntityId;
	int_t delayTemptCounter;
	bool running;
	int_t breedingFood;
	bool scaredByPlayerMovement;
	bool oldAvoidsWater;
};
