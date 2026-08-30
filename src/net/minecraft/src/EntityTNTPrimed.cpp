#include "EntityTNTPrimed.h"
#include "java/Arithmetic.h"
#include "java/Math.h"

#include <cmath>

#include "MathHelper.h"
#include "NBTTagCompound.h"
#include "World.h"
#include "Explosion.h"

EntityTNTPrimed::EntityTNTPrimed(World *world)
	: Entity(world)
{
	ensureEntityInit();
	fuse = 0;
	preventEntitySpawning = true;
	setSize(0.98f, 0.98f);
	yOffset = height / 2.0f;
}

EntityTNTPrimed::EntityTNTPrimed(World *world, double d, double d1, double d2)
	: EntityTNTPrimed(world)
{
	setPosition(d, d1, d2);
	float f = (float)(Math::random() * 3.1415927410125732 * 2.0);
	motionX = -(float)JavaMath::sin(static_cast<double>(f)) * 0.02f;
	motionY = 0.20000000298023224;
	motionZ = -(float)JavaMath::cos(static_cast<double>(f)) * 0.02f;
	fuse = 80;
	prevPosX = d;
	prevPosY = d1;
	prevPosZ = d2;
}

void EntityTNTPrimed::entityInit()
{
}

bool EntityTNTPrimed::canTriggerWalking()
{
	return false;
}

bool EntityTNTPrimed::canBeCollidedWith()
{
	return !isDead;
}

void EntityTNTPrimed::onUpdate()
{
	prevPosX = posX;
	prevPosY = posY;
	prevPosZ = posZ;
	motionY -= 0.039999999105930328;
	moveEntity(motionX, motionY, motionZ);
	motionX *= 0.98000001907348633;
	motionY *= 0.98000001907348633;
	motionZ *= 0.98000001907348633;
	if (onGround)
	{
		motionX *= 0.69999998807907104;
		motionZ *= 0.69999998807907104;
		motionY *= -0.5;
	}
	if (fuse-- <= 0)
	{
		if (!worldObj->multiplayerWorld)
		{
			setEntityDead();
			explode();
		}
		else
		{
			setEntityDead();
		}
	}
	else
	{
		worldObj->spawnParticle("smoke", posX, posY + 0.5, posZ, 0.0, 0.0, 0.0);
	}
}

void EntityTNTPrimed::explode()
{
	float f = 4.0f;
	delete worldObj->createExplosion(nullptr, posX, posY, posZ, f);
}

void EntityTNTPrimed::writeEntityToNBT(NBTTagCompound *nbttagcompound)
{
	nbttagcompound->setByte("Fuse", JavaArithmetic::byteFromBits(static_cast<ubyte_t>(fuse)));
}

void EntityTNTPrimed::readEntityFromNBT(NBTTagCompound *nbttagcompound)
{
	fuse = nbttagcompound->getByte("Fuse");
}

float EntityTNTPrimed::getShadowSize()
{
	return 0.0f;
}
