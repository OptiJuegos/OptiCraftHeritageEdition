#include "EntityAIPanic.h"

#include "EntityCreature.h"
#include "PathNavigate.h"
#include "RandomPositionGenerator.h"
#include "Vec3D.h"

EntityAIPanic::EntityAIPanic(EntityCreature *entity, float moveSpeed)
	: theEntity(entity), speed(moveSpeed), targetX(0.0), targetY(0.0), targetZ(0.0)
{
	setMutexBits(1);
}

bool EntityAIPanic::shouldExecute()
{
	if (theEntity == nullptr || theEntity->getAITarget() == nullptr)
		return false;
	Vec3D *target = RandomPositionGenerator::findRandomTarget(theEntity, 5, 4);
	if (target == nullptr)
		return false;
	targetX = target->xCoord;
	targetY = target->yCoord;
	targetZ = target->zCoord;
	return true;
}

bool EntityAIPanic::continueExecuting()
{
	return theEntity != nullptr && theEntity->getNavigator() != nullptr && !theEntity->getNavigator()->noPath();
}

void EntityAIPanic::startExecuting()
{
	if (theEntity != nullptr && theEntity->getNavigator() != nullptr)
		theEntity->getNavigator()->tryMoveToXYZ(targetX, targetY, targetZ, speed);
}
