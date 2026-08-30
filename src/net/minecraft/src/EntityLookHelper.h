#pragma once

#include "platform/PlatformTuning.h"

class Entity;
class EntityLiving;

// net.minecraft.src.EntityLookHelper
class EntityLookHelper
{
public:
	explicit EntityLookHelper(EntityLiving *entity);

	void setLookPositionWithEntity(Entity *target, float yawSpeed, float pitchSpeed);
	void setLookPosition(double x, double y, double z, float yawSpeed, float pitchSpeed);
	void onUpdateLook();

private:
#if PLATFORM_FLOAT_ENTITY_AI_MATH
	using look_math_t = float;
#else
	using look_math_t = double;
#endif

	float updateRotation(float current, float target, float maxChange) const;

	EntityLiving *entity;
	float yawSpeed;
	float pitchSpeed;
	bool isLooking;
	look_math_t posX;
	look_math_t posY;
	look_math_t posZ;
};
