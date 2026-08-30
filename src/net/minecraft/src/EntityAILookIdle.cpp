#include "EntityAILookIdle.h"
#include "java/Math.h"

#include <cmath>

#include "EntityLiving.h"
#include "EntityLookHelper.h"

EntityAILookIdle::EntityAILookIdle(EntityLiving *entity)
	: idleEntity(entity), lookX(0.0f), lookZ(0.0f), idleTime(0)
{
	setMutexBits(3);
}

bool EntityAILookIdle::shouldExecute()
{
	return idleEntity != nullptr && idleEntity->getRNG().nextFloat() < 0.02f;
}

bool EntityAILookIdle::continueExecuting()
{
	return idleEntity != nullptr && !idleEntity->isDead && idleTime >= 0;
}

void EntityAILookIdle::startExecuting()
{
	if (idleEntity == nullptr)
		return;
#if PLATFORM_FLOAT_ENTITY_AI_MATH
	const float angle = idleEntity->getRNG().nextDoubleFloat() * 3.1415927f * 2.0f;
	lookX = std::cos(angle);
	lookZ = std::sin(angle);
#else
	double angle = 3.14159265358979323846 * 2.0 * idleEntity->getRNG().nextDouble();
	lookX = JavaMath::cos(angle);
	lookZ = JavaMath::sin(angle);
#endif
	idleTime = 20 + idleEntity->getRNG().nextInt(20);
}

void EntityAILookIdle::updateTask()
{
	if (idleEntity == nullptr || idleEntity->getLookHelper() == nullptr)
		return;
	--idleTime;
	idleEntity->getLookHelper()->setLookPosition(idleEntity->posX + lookX,
		idleEntity->posY + (double)idleEntity->getEyeHeight(), idleEntity->posZ + lookZ,
		10.0f, (float)idleEntity->getVerticalFaceSpeed());
}
