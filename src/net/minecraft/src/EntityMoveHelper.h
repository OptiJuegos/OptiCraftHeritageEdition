#pragma once

#include "platform/PlatformTuning.h"

class EntityLiving;

// net.minecraft.src.EntityMoveHelper
class EntityMoveHelper
{
public:
	explicit EntityMoveHelper(EntityLiving *entity);

	bool isUpdating() const;
	float getSpeed() const;
	void setMoveTo(double x, double y, double z, float speed);
	void onUpdateMoveHelper();

private:
#if PLATFORM_FLOAT_ENTITY_AI_MATH
	using move_math_t = float;
#else
	using move_math_t = double;
#endif

	float limitAngle(float current, float target, float maxChange) const;

	EntityLiving *entity;
	move_math_t posX;
	move_math_t posY;
	move_math_t posZ;
	float speed;
	bool update;
};
