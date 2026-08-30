#include "EntityLookHelper.h"

#include <cmath>

#include "AxisAlignedBB.h"
#include "Entity.h"
#include "EntityLiving.h"
#include "PathNavigate.h"
#include "MathHelper.h"
#include "java/Math.h"

EntityLookHelper::EntityLookHelper(EntityLiving *entity)
	: entity(entity), yawSpeed(0.0f), pitchSpeed(0.0f), isLooking(false),
	  posX(static_cast<look_math_t>(0.0)), posY(static_cast<look_math_t>(0.0)), posZ(static_cast<look_math_t>(0.0))
{
}

void EntityLookHelper::setLookPositionWithEntity(Entity *target, float newYawSpeed, float newPitchSpeed)
{
	if (target == nullptr)
		return;
	posX = static_cast<look_math_t>(target->posX);
	EntityLiving *living = dynamic_cast<EntityLiving *>(target);
	if (living != nullptr)
		posY = static_cast<look_math_t>(target->posY) + static_cast<look_math_t>(living->getEyeHeight());
	else
		posY = static_cast<look_math_t>((target->boundingBox->minY + target->boundingBox->maxY) * 0.5);
	posZ = static_cast<look_math_t>(target->posZ);
	yawSpeed = newYawSpeed;
	pitchSpeed = newPitchSpeed;
	isLooking = true;
}

void EntityLookHelper::setLookPosition(double x, double y, double z, float newYawSpeed, float newPitchSpeed)
{
	posX = static_cast<look_math_t>(x);
	posY = static_cast<look_math_t>(y);
	posZ = static_cast<look_math_t>(z);
	yawSpeed = newYawSpeed;
	pitchSpeed = newPitchSpeed;
	isLooking = true;
}

void EntityLookHelper::onUpdateLook()
{
	if (entity == nullptr)
		return;
	entity->rotationPitch = 0.0f;
	if (isLooking)
	{
		isLooking = false;
#if PLATFORM_FLOAT_ENTITY_AI_MATH
		const float dx = posX - static_cast<float>(entity->posX);
		const float dy = posY - (static_cast<float>(entity->posY) + entity->getEyeHeight());
		const float dz = posZ - static_cast<float>(entity->posZ);
		const float horizontal = MathHelper::sqrt_float(dx * dx + dz * dz);
		const float targetYaw = std::atan2(dz, dx) * (180.0f / 3.1415927410125732f) - 90.0f;
		const float targetPitch = -std::atan2(dy, horizontal) * (180.0f / 3.1415927410125732f);
#else
		const double dx = posX - entity->posX;
		const double dy = posY - (entity->posY + static_cast<double>(entity->getEyeHeight()));
		const double dz = posZ - entity->posZ;
		const double horizontal = static_cast<double>(MathHelper::sqrt_double(dx * dx + dz * dz));
		const float targetYaw = static_cast<float>(JavaMath::atan2(dz, dx) * 180.0 / 3.1415927410125732) - 90.0f;
		const float targetPitch = static_cast<float>(-(JavaMath::atan2(dy, horizontal) * 180.0 / 3.1415927410125732));
#endif
		entity->rotationPitch = updateRotation(entity->rotationPitch, targetPitch, pitchSpeed);
		entity->rotationYawHead = updateRotation(entity->rotationYawHead, targetYaw, yawSpeed);
	}
	else
	{
		entity->rotationYawHead = updateRotation(entity->rotationYawHead, entity->renderYawOffset, 10.0f);
	}

	float delta;
	for (delta = entity->rotationYawHead - entity->renderYawOffset; delta < -180.0f; delta += 360.0f) {}
	while (delta >= 180.0f) delta -= 360.0f;
	PathNavigate *navigator = entity->getNavigator();
	if (navigator != nullptr && !navigator->noPath())
	{
		if (delta < -75.0f)
			entity->rotationYawHead = entity->renderYawOffset - 75.0f;
		if (delta > 75.0f)
			entity->rotationYawHead = entity->renderYawOffset + 75.0f;
	}
}

float EntityLookHelper::updateRotation(float current, float target, float maxChange) const
{
	float delta;
	for (delta = target - current; delta < -180.0f; delta += 360.0f) {}
	while (delta >= 180.0f) delta -= 360.0f;
	if (delta > maxChange) delta = maxChange;
	if (delta < -maxChange) delta = -maxChange;
	return current + delta;
}
