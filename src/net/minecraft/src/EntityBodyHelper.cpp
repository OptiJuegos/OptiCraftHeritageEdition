#include "EntityBodyHelper.h"

#include <algorithm>
#include <cmath>

#include "EntityLiving.h"

EntityBodyHelper::EntityBodyHelper(EntityLiving *entity)
	: entity(entity), rotationTickCounter(0), prevHeadYaw(0.0f)
{
}

void EntityBodyHelper::updateRenderAngles()
{
	if (entity == nullptr)
		return;
	double dx = entity->posX - entity->prevPosX;
	double dz = entity->posZ - entity->prevPosZ;
	if (dx * dx + dz * dz > static_cast<double>(2.5000003e-7f))
	{
		entity->renderYawOffset = entity->rotationYaw;
		entity->rotationYawHead = computeAngleWithBound(entity->renderYawOffset, entity->rotationYawHead, 75.0f);
		prevHeadYaw = entity->rotationYawHead;
		rotationTickCounter = 0;
		return;
	}

	float maxChange = 75.0f;
	if (std::fabs(entity->rotationYawHead - prevHeadYaw) > 15.0f)
	{
		rotationTickCounter = 0;
		prevHeadYaw = entity->rotationYawHead;
	}
	else
	{
		++rotationTickCounter;
		if (rotationTickCounter > 10)
			maxChange = std::max(1.0f - (float)(rotationTickCounter - 10) / 10.0f, 0.0f) * 75.0f;
	}
	entity->renderYawOffset = computeAngleWithBound(entity->rotationYawHead, entity->renderYawOffset, maxChange);
}

float EntityBodyHelper::computeAngleWithBound(float target, float current, float maxChange) const
{
	float delta;
	for (delta = target - current; delta < -180.0f; delta += 360.0f) {}
	while (delta >= 180.0f) delta -= 360.0f;
	if (delta < -maxChange) delta = -maxChange;
	if (delta >= maxChange) delta = maxChange;
	return target - delta;
}
