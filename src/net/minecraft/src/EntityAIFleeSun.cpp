#include "EntityAIFleeSun.h"

#include "EntityCreature.h"
#include "MathHelper.h"
#include "PathNavigate.h"
#include "Vec3D.h"
#include "World.h"

EntityAIFleeSun::EntityAIFleeSun(EntityCreature *entity, float moveSpeed)
	: theCreature(entity), theWorld(entity != nullptr ? entity->worldObj : nullptr),
	  shelterX(0.0), shelterY(0.0), shelterZ(0.0), speed(moveSpeed)
{
	setMutexBits(1);
}

bool EntityAIFleeSun::shouldExecute()
{
	if (theCreature == nullptr || theWorld == nullptr || !theWorld->isDaytime() || !theCreature->isBurning())
		return false;
	if (!theWorld->canBlockSeeTheSky(MathHelper::floor_double(theCreature->posX),
		(int_t)theCreature->boundingBox->minY, MathHelper::floor_double(theCreature->posZ)))
		return false;
	Vec3D *shelter = findPossibleShelter();
	if (shelter == nullptr)
		return false;
	shelterX = shelter->xCoord;
	shelterY = shelter->yCoord;
	shelterZ = shelter->zCoord;
	return true;
}

bool EntityAIFleeSun::continueExecuting()
{
	return theCreature != nullptr && theCreature->getNavigator() != nullptr && !theCreature->getNavigator()->noPath();
}

void EntityAIFleeSun::startExecuting()
{
	if (theCreature != nullptr && theCreature->getNavigator() != nullptr)
		theCreature->getNavigator()->tryMoveToXYZ(shelterX, shelterY, shelterZ, speed);
}

Vec3D *EntityAIFleeSun::findPossibleShelter()
{
	if (theCreature == nullptr || theWorld == nullptr)
		return nullptr;
	Random &random = theCreature->getRNG();
	for (int_t i = 0; i < 10; ++i)
	{
		int_t x = MathHelper::floor_double(theCreature->posX + (double)random.nextInt(20) - 10.0);
		int_t y = MathHelper::floor_double(theCreature->boundingBox->minY + (double)random.nextInt(6) - 3.0);
		int_t z = MathHelper::floor_double(theCreature->posZ + (double)random.nextInt(20) - 10.0);
		if (!theWorld->canBlockSeeTheSky(x, y, z) && theCreature->getBlockPathWeight(x, y, z) < 0.0f)
			return Vec3D::createVector((double)x, (double)y, (double)z);
	}
	return nullptr;
}
