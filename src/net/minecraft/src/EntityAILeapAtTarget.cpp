#include "EntityAILeapAtTarget.h"

#include "EntityLiving.h"
#include "MathHelper.h"
#include "World.h"
#include "platform/PlatformTuning.h"

EntityAILeapAtTarget::EntityAILeapAtTarget(EntityLiving *entity, float verticalMotion)
	: leaper(entity), targetEntityId(-1), leapMotionY(verticalMotion)
{
	setMutexBits(5);
}

EntityLiving *EntityAILeapAtTarget::resolveTarget() const
{
	if (leaper == nullptr || leaper->worldObj == nullptr || targetEntityId < 0)
		return nullptr;
	return dynamic_cast<EntityLiving *>(leaper->worldObj->getEntityByID(targetEntityId));
}

bool EntityAILeapAtTarget::shouldExecute()
{
	targetEntityId = -1;
	if (leaper == nullptr)
		return false;
	EntityLiving *target = leaper->getAttackTarget();
	if (target == nullptr)
		return false;
	double distanceSq = leaper->getDistanceSqToEntity(target);
	if (distanceSq < 4.0 || distanceSq > 16.0 || !leaper->onGround || leaper->getRNG().nextInt(5) != 0)
		return false;
	targetEntityId = target->entityId;
	return true;
}

bool EntityAILeapAtTarget::continueExecuting()
{
	return leaper != nullptr && !leaper->onGround;
}

void EntityAILeapAtTarget::startExecuting()
{
	EntityLiving *target = resolveTarget();
	if (leaper == nullptr || target == nullptr)
		return;
#if PLATFORM_FLOAT_ENTITY_AI_MATH
	const float dx = static_cast<float>(target->posX - leaper->posX);
	const float dz = static_cast<float>(target->posZ - leaper->posZ);
	const float length = MathHelper::sqrt_float(dx * dx + dz * dz);
	if (length <= 1.0e-7f)
		return;
	leaper->motionX += static_cast<double>(dx / length * 0.4f) + leaper->motionX * 0.20000000298023224;
	leaper->motionZ += static_cast<double>(dz / length * 0.4f) + leaper->motionZ * 0.20000000298023224;
#else
	const double dx = target->posX - leaper->posX;
	const double dz = target->posZ - leaper->posZ;
	const float length = MathHelper::sqrt_double(dx * dx + dz * dz);
	if (length <= 1.0e-7f)
		return;
	leaper->motionX += dx / (double)length * 0.5 * static_cast<double>(0.8f) + leaper->motionX * static_cast<double>(0.2f);
	leaper->motionZ += dz / (double)length * 0.5 * static_cast<double>(0.8f) + leaper->motionZ * static_cast<double>(0.2f);
#endif
	leaper->motionY = static_cast<double>(leapMotionY);
}

void EntityAILeapAtTarget::resetTask()
{
	targetEntityId = -1;
}
