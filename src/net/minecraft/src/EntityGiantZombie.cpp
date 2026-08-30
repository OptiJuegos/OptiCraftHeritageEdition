#include "EntityGiantZombie.h"

#include "World.h"

EntityGiantZombie::EntityGiantZombie(World *world)
	: EntityMob(world)
{
	texture = "/mob/zombie.png";
	moveSpeed = 0.5f;
	attackStrength = 50;
	setHealth(getMaxHealth());
	yOffset *= 6.0f;
	setSize(width * 6.0f, height * 6.0f);
}

int_t EntityGiantZombie::getMaxHealth() const
{
	return 100;
}

float EntityGiantZombie::getBlockPathWeight(int_t i, int_t j, int_t k)
{
	return worldObj->getLightBrightness(i, j, k) - 0.5f;
}
