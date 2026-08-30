#include "EntityEnderCrystal.h"

#include "Block.h"
#include "BlockFire.h"
#include "DamageSource.h"
#include "DataWatcher.h"
#include "MathHelper.h"
#include "NBTTagCompound.h"
#include "World.h"
#include "Explosion.h"

EntityEnderCrystal::EntityEnderCrystal(World *world)
	: Entity(world), innerRotation(0), health(5)
{
	ensureEntityInit();
	preventEntitySpawning = true;
	setSize(2.0f, 2.0f);
	yOffset = height / 2.0f;
	innerRotation = rand.nextInt(100000);
}

EntityEnderCrystal::EntityEnderCrystal(World *world, double x, double y, double z)
	: EntityEnderCrystal(world)
{
	setPosition(x, y, z);
}

bool EntityEnderCrystal::canTriggerWalking()
{
	return false;
}

void EntityEnderCrystal::entityInit()
{
	if (!dataWatcher->hasObject(8))
		dataWatcher->addObject(8, static_cast<int_t>(0));
}

void EntityEnderCrystal::onUpdate()
{
	prevPosX = posX;
	prevPosY = posY;
	prevPosZ = posZ;
	++innerRotation;
	dataWatcher->updateObject(8, static_cast<int_t>(health));

	const int_t x = MathHelper::floor_double(posX);
	const int_t y = MathHelper::floor_double(posY);
	const int_t z = MathHelper::floor_double(posZ);
	if (Block::fire != nullptr && worldObj->getBlockId(x, y, z) != Block::fire->blockID)
		worldObj->setBlockWithNotify(x, y, z, Block::fire->blockID);
}

void EntityEnderCrystal::writeEntityToNBT(NBTTagCompound *)
{
}

void EntityEnderCrystal::readEntityFromNBT(NBTTagCompound *)
{
}

float EntityEnderCrystal::getShadowSize()
{
	return 0.0f;
}

bool EntityEnderCrystal::canBeCollidedWith()
{
	return true;
}

bool EntityEnderCrystal::attackEntityFrom(const DamageSource &, int_t)
{
	if (!isDead && !worldObj->multiplayerWorld)
	{
		health = 0;
		if (health <= 0)
		{
			setEntityDead();
			delete worldObj->createExplosion(nullptr, posX, posY, posZ, 6.0f);
		}
	}
	return true;
}
