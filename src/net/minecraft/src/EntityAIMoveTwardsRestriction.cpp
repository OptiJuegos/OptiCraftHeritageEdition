#include "EntityAIMoveTwardsRestriction.h"

#include "ChunkCoordinates.h"
#include "EntityCreature.h"
#include "PathNavigate.h"
#include "RandomPositionGenerator.h"
#include "Vec3D.h"

EntityAIMoveTwardsRestriction::EntityAIMoveTwardsRestriction(EntityCreature *entity, float speed)
	: theEntity(entity), movePosX(0.0), movePosY(0.0), movePosZ(0.0), speed(speed)
{
	setMutexBits(1);
}

bool EntityAIMoveTwardsRestriction::shouldExecute()
{
	if (theEntity == nullptr || theEntity->isWithinHomeDistanceCurrentPosition())
		return false;
	ChunkCoordinates home = theEntity->getHomePosition();
	Vec3D *homeVector = Vec3D::createVector((double)home.x, (double)home.y, (double)home.z);
	Vec3D *target = RandomPositionGenerator::findRandomTargetBlockTowards(theEntity, 16, 7, homeVector);
	if (target == nullptr)
		return false;
	movePosX = target->xCoord;
	movePosY = target->yCoord;
	movePosZ = target->zCoord;
	return true;
}

bool EntityAIMoveTwardsRestriction::continueExecuting()
{
	return theEntity != nullptr && theEntity->getNavigator() != nullptr && !theEntity->getNavigator()->noPath();
}

void EntityAIMoveTwardsRestriction::startExecuting()
{
	if (theEntity != nullptr && theEntity->getNavigator() != nullptr)
		theEntity->getNavigator()->tryMoveToXYZ(movePosX, movePosY, movePosZ, speed);
}
