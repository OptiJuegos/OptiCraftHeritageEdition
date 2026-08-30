#include "EntityWaterMob.h"

#include "World.h"

bool EntityWaterMob::getCanSpawnHere()
{
	return worldObj->checkIfAABBIsClear(boundingBox);
}

int_t EntityWaterMob::getExperiencePoints(EntityPlayer *player)
{
	(void)player;
	return 1 + worldObj->rand.nextInt(3);
}
