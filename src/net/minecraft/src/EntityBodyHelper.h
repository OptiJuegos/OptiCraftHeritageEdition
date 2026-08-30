#pragma once

#include "java/Type.h"

class EntityLiving;

// net.minecraft.src.EntityBodyHelper
class EntityBodyHelper
{
public:
	explicit EntityBodyHelper(EntityLiving *entity);
	void updateRenderAngles();

private:
	float computeAngleWithBound(float target, float current, float maxChange) const;
	EntityLiving *entity;
	int_t rotationTickCounter;
	float prevHeadYaw;
};
