#include "EntityMoveHelper.h"

#include <cmath>

#include "AxisAlignedBB.h"
#include "EntityJumpHelper.h"
#include "EntityLiving.h"
#include "MathHelper.h"
#include "java/Math.h"

EntityMoveHelper::EntityMoveHelper(EntityLiving *entity)
	: entity(entity), posX(entity != nullptr ? static_cast<move_math_t>(entity->posX) : static_cast<move_math_t>(0.0)),
	  posY(entity != nullptr ? static_cast<move_math_t>(entity->posY) : static_cast<move_math_t>(0.0)),
	  posZ(entity != nullptr ? static_cast<move_math_t>(entity->posZ) : static_cast<move_math_t>(0.0)),
	  speed(0.0f), update(false)
{
}

bool EntityMoveHelper::isUpdating() const
{
	return update;
}

float EntityMoveHelper::getSpeed() const
{
	return speed;
}

void EntityMoveHelper::setMoveTo(double x, double y, double z, float newSpeed)
{
	posX = static_cast<move_math_t>(x);
	posY = static_cast<move_math_t>(y);
	posZ = static_cast<move_math_t>(z);
	speed = newSpeed;
	update = true;
}

void EntityMoveHelper::onUpdateMoveHelper()
{
	if (entity == nullptr)
		return;
	entity->setMoveForward(0.0f);
	if (!update)
		return;
	update = false;
	int_t baseY = MathHelper::floor_double(entity->boundingBox->minY + 0.5);
#if PLATFORM_FLOAT_ENTITY_AI_MATH
	const float dx = posX - static_cast<float>(entity->posX);
	const float dz = posZ - static_cast<float>(entity->posZ);
	const float dy = posY - static_cast<float>(baseY);
	const float distanceSq = dx * dx + dy * dy + dz * dz;
	if (distanceSq < 2.5000003e-7f)
		return;
	const float targetYaw = std::atan2(dz, dx) * (180.0f / 3.1415927410125732f) - 90.0f;
#else
	const double dx = posX - entity->posX;
	const double dz = posZ - entity->posZ;
	const double dy = posY - static_cast<double>(baseY);
	const double distanceSq = dx * dx + dy * dy + dz * dz;
	if (distanceSq < static_cast<double>(2.5000003e-7f))
		return;
	const float targetYaw = static_cast<float>(JavaMath::atan2(dz, dx) * 180.0 / 3.1415927410125732) - 90.0f;
#endif
	entity->rotationYaw = limitAngle(entity->rotationYaw, targetYaw, 30.0f);
	entity->setAIMoveSpeed(speed);
#if PLATFORM_FLOAT_ENTITY_AI_MATH
	if (dy > 0.0f && dx * dx + dz * dz < 1.0f)
#else
	if (dy > 0.0 && dx * dx + dz * dz < 1.0)
#endif
	{
		EntityJumpHelper *jumpHelper = entity->getJumpHelper();
		if (jumpHelper != nullptr)
			jumpHelper->setJumping();
	}
}

float EntityMoveHelper::limitAngle(float current, float target, float maxChange) const
{
	float delta;
	for (delta = target - current; delta < -180.0f; delta += 360.0f) {}
	while (delta >= 180.0f) delta -= 360.0f;
	if (delta > maxChange) delta = maxChange;
	if (delta < -maxChange) delta = -maxChange;
	return current + delta;
}
