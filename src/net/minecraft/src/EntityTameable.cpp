#include "EntityTameable.h"

#include "DataWatcher.h"
#include "EntityAISit.h"
#include "EntityLiving.h"
#include "EntityPlayer.h"
#include "NBTTagCompound.h"
#include "World.h"

EntityTameable::EntityTameable(World *world)
	: EntityAnimal(world), aiSit(new EntityAISit(this))
{
}

void EntityTameable::entityInit()
{
	EntityAnimal::entityInit();
	dataWatcher->addObject(16, (byte_t)0);
	dataWatcher->addObject(17, std::string());
}

void EntityTameable::writeEntityToNBT(NBTTagCompound *compound)
{
	EntityAnimal::writeEntityToNBT(compound);
	compound->setString("Owner", getOwnerName());
	compound->setBoolean("Sitting", isSitting());
}

void EntityTameable::readEntityFromNBT(NBTTagCompound *compound)
{
	EntityAnimal::readEntityFromNBT(compound);
	jstring owner = compound->getString("Owner");
	if (!owner.empty())
	{
		setOwner(owner);
		setTamed(true);
	}
	if (aiSit != nullptr)
		aiSit->setSitting(compound->getBoolean("Sitting"));
}

void EntityTameable::showHeartsOrSmokeFX(bool success)
{
	const char *particle = success ? "heart" : "smoke";
	for (int_t i = 0; i < 7; ++i)
	{
		double motionX = rand.nextGaussian() * 0.02;
		double motionY = rand.nextGaussian() * 0.02;
		double motionZ = rand.nextGaussian() * 0.02;
		const double particleX = posX + (double)(rand.nextFloat() * width * 2.0f) - (double)width;
		const double particleY = posY + 0.5 + (double)(rand.nextFloat() * height);
		const double particleZ = posZ + (double)(rand.nextFloat() * width * 2.0f) - (double)width;
		worldObj->spawnParticle(particle, particleX, particleY, particleZ, motionX, motionY, motionZ);
	}
}

void EntityTameable::handleHealthUpdate(byte_t state)
{
	if (state == 7)
		showHeartsOrSmokeFX(true);
	else if (state == 6)
		showHeartsOrSmokeFX(false);
	else
		EntityAnimal::handleHealthUpdate(state);
}

bool EntityTameable::isTamed() const
{
	return (dataWatcher->getWatchableObjectByte(16) & 4) != 0;
}

void EntityTameable::setTamed(bool value)
{
	byte_t flags = dataWatcher->getWatchableObjectByte(16);
	dataWatcher->updateObject(16, value ? (byte_t)(flags | 4) : (byte_t)(flags & 0xfb));
}

bool EntityTameable::isSitting() const
{
	return (dataWatcher->getWatchableObjectByte(16) & 1) != 0;
}

void EntityTameable::setSitting(bool value)
{
	byte_t flags = dataWatcher->getWatchableObjectByte(16);
	dataWatcher->updateObject(16, value ? (byte_t)(flags | 1) : (byte_t)(flags & 0xfe));
}

jstring EntityTameable::getOwnerName() const
{
	return dataWatcher->getWatchableObjectString(17);
}

void EntityTameable::setOwner(const jstring &owner)
{
	dataWatcher->updateObject(17, std::string(owner));
}

EntityLiving *EntityTameable::getOwner() const
{
	if (worldObj == nullptr)
		return nullptr;
	return worldObj->getPlayerEntityByName(getOwnerName());
}

EntityAISit *EntityTameable::getAISit() const
{
	return aiSit;
}
