#include "EntityAgeable.h"

#include "DataWatcher.h"
#include "NBTTagCompound.h"

EntityAgeable::EntityAgeable(World *world)
	: EntityCreature(world)
{
}

void EntityAgeable::entityInit()
{
	EntityLiving::entityInit();
	dataWatcher->addObject(12, int_t(0));
}

int_t EntityAgeable::getGrowingAge()
{
	return dataWatcher->getWatchableObjectInt(12);
}

void EntityAgeable::setGrowingAge(int_t age)
{
	dataWatcher->updateObject(12, age);
}

void EntityAgeable::writeEntityToNBT(NBTTagCompound *tag)
{
	EntityCreature::writeEntityToNBT(tag);
	tag->setInteger("Age", getGrowingAge());
}

void EntityAgeable::readEntityFromNBT(NBTTagCompound *tag)
{
	EntityCreature::readEntityFromNBT(tag);
	setGrowingAge(tag->getInteger("Age"));
}

void EntityAgeable::onLivingUpdate()
{
	EntityCreature::onLivingUpdate();
	int_t age = getGrowingAge();
	if (age < 0)
		setGrowingAge(age + 1);
	else if (age > 0)
		setGrowingAge(age - 1);
}

bool EntityAgeable::isChild()
{
	return getGrowingAge() < 0;
}
