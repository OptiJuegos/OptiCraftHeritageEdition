#pragma once

#include "EntityAIBase.h"
#include "platform/PlatformTuning.h"

class EntityLiving;

// net.minecraft.src.EntityAILookIdle
class EntityAILookIdle : public EntityAIBase
{
public:
	explicit EntityAILookIdle(EntityLiving *entity);

	bool shouldExecute() override;
	bool continueExecuting() override;
	void startExecuting() override;
	void updateTask() override;

private:
	EntityLiving *idleEntity;
#if PLATFORM_FLOAT_ENTITY_AI_MATH
	float lookX;
	float lookZ;
#else
	double lookX;
	double lookZ;
#endif
	int_t idleTime;
};
