#include "EntityAIWander.h"

#include "EntityCreature.h"
#include "PathNavigate.h"
#include "RandomPositionGenerator.h"
#include "Vec3D.h"

EntityAIWander::EntityAIWander(EntityCreature *entity, float speed)
	: entity(entity), moveX(0.0), moveY(0.0), moveZ(0.0), speed(speed)
{
	setMutexBits(1);
}

bool EntityAIWander::shouldExecute()
{
	if (entity == nullptr || entity->getAge() >= 100 || entity->getRNG().nextInt(120) != 0)
		return false;
	Vec3D *target = RandomPositionGenerator::findRandomTarget(entity, 10, 7);
	if (target == nullptr)
		return false;
	moveX = target->xCoord;
	moveY = target->yCoord;
	moveZ = target->zCoord;
	return true;
}

bool EntityAIWander::continueExecuting()
{
	return entity != nullptr && entity->getNavigator() != nullptr && !entity->getNavigator()->noPath();
}

void EntityAIWander::startExecuting()
{
	if (entity != nullptr && entity->getNavigator() != nullptr)
		entity->getNavigator()->tryMoveToXYZ(moveX, moveY, moveZ, speed);
}
