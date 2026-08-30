#include "EntityGolem.h"

#include "NBTTagCompound.h"

EntityGolem::EntityGolem(World *world)
	: EntityCreature(world)
{
}

void EntityGolem::fall(float distance)
{
	(void)distance;
}

void EntityGolem::writeEntityToNBT(NBTTagCompound *tag)
{
	EntityCreature::writeEntityToNBT(tag);
}

void EntityGolem::readEntityFromNBT(NBTTagCompound *tag)
{
	EntityCreature::readEntityFromNBT(tag);
}

jstring EntityGolem::getLivingSound()
{
	return "none";
}

jstring EntityGolem::getHurtSound()
{
	return "none";
}

jstring EntityGolem::getDeathSound()
{
	return "none";
}

int_t EntityGolem::getTalkInterval()
{
	return 120;
}

bool EntityGolem::canDespawn()
{
	return false;
}
