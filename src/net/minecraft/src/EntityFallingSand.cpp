#include "EntityFallingSand.h"
#include "java/Arithmetic.h"

#include "Block.h"
#include "BlockPistonMoving.h"
#include "BlockSand.h"
#include "MathHelper.h"
#include "NBTTagCompound.h"
#include "World.h"

EntityFallingSand::EntityFallingSand(World *world)
	: Entity(world)
{
	ensureEntityInit();
	fallTime = 0;
}

EntityFallingSand::EntityFallingSand(World *world, double d, double d1, double d2, int_t i)
	: Entity(world)
{
	ensureEntityInit();
	fallTime = 0;
	blockID = i;
	preventEntitySpawning = true;
	setSize(0.98f, 0.98f);
	yOffset = height / 2.0f;
	setPosition(d, d1, d2);
	motionX = 0.0;
	motionY = 0.0;
	motionZ = 0.0;
	prevPosX = d;
	prevPosY = d1;
	prevPosZ = d2;
}

bool EntityFallingSand::canTriggerWalking()
{
	return false;
}

void EntityFallingSand::entityInit()
{
}

bool EntityFallingSand::canBeCollidedWith()
{
	return !isDead;
}

void EntityFallingSand::onUpdate()
{
	if (blockID == 0)
	{
		setEntityDead();
		return;
	}

	prevPosX = posX;
	prevPosY = posY;
	prevPosZ = posZ;
	++fallTime;
	motionY -= 0.039999999105930328;
	moveEntity(motionX, motionY, motionZ);
	motionX *= 0.98000001907348633;
	motionY *= 0.98000001907348633;
	motionZ *= 0.98000001907348633;

	int_t i = MathHelper::floor_double(posX);
	int_t j = MathHelper::floor_double(posY);
	int_t k = MathHelper::floor_double(posZ);
	if (fallTime == 1 && worldObj->getBlockId(i, j, k) == blockID)
	{
		worldObj->setBlockWithNotify(i, j, k, 0);
	}
	else if (!worldObj->multiplayerWorld && fallTime == 1)
	{
		setEntityDead();
	}

	if (onGround)
	{
		motionX *= 0.69999998807907104;
		motionZ *= 0.69999998807907104;
		motionY *= -0.5;
		if (worldObj->getBlockId(i, j, k) != Block::pistonMoving->blockID)
		{
			setEntityDead();
			if ((!worldObj->canBlockBePlacedAt(blockID, i, j, k, true, 1) ||
			     BlockSand::canFallBelow(worldObj, i, j - 1, k) ||
			     !worldObj->setBlockWithNotify(i, j, k, blockID)) && !worldObj->multiplayerWorld)
			{
				dropItem(blockID, 1);
			}
		}
	}
	else if ((fallTime > 100 && !worldObj->multiplayerWorld && (j < 1 || j > 256)) || fallTime > 600)
	{
		dropItem(blockID, 1);
		setEntityDead();
	}
}

void EntityFallingSand::writeEntityToNBT(NBTTagCompound *nbttagcompound)
{
	nbttagcompound->setByte("Tile", JavaArithmetic::byteFromBits(static_cast<ubyte_t>(blockID)));
}

void EntityFallingSand::readEntityFromNBT(NBTTagCompound *nbttagcompound)
{
	blockID = nbttagcompound->getByte("Tile") & 0xFF;
}

float EntityFallingSand::getShadowSize()
{
	return 0.0f;
}
