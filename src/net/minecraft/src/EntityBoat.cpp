#include "EntityBoat.h"
#include "DamageSource.h"
#include "DataWatcher.h"
#include "java/Math.h"
#include "java/Arithmetic.h"

#include <cmath>

#include "AxisAlignedBB.h"
#include "Block.h"
#include "EntityPlayer.h"
#include "Item.h"
#include "MathHelper.h"
#include "NBTTagCompound.h"
#include "World.h"
#include "Material.h"

EntityBoat::EntityBoat(World *world)
	: Entity(world)
{
	ensureEntityInit();
	boatTicks = 0;
	boatX = 0.0;
	boatY = 0.0;
	boatZ = 0.0;
	boatYaw = 0.0;
	boatPitch = 0.0;
	velocityX = 0.0;
	velocityY = 0.0;
	velocityZ = 0.0;
	preventEntitySpawning = true;
	setSize(1.5f, 0.6f);
	yOffset = height / 2.0f;
}

EntityBoat::EntityBoat(World *world, double d, double d1, double d2)
	: Entity(world)
{
	ensureEntityInit();
	boatTicks = 0;
	boatX = 0.0;
	boatY = 0.0;
	boatZ = 0.0;
	boatYaw = 0.0;
	boatPitch = 0.0;
	velocityX = 0.0;
	velocityY = 0.0;
	velocityZ = 0.0;
	preventEntitySpawning = true;
	setSize(1.5f, 0.6f);
	yOffset = height / 2.0f;
	setPosition(d, d1 + (double)yOffset, d2);
	motionX = 0.0;
	motionY = 0.0;
	motionZ = 0.0;
	prevPosX = d;
	prevPosY = d1;
	prevPosZ = d2;
}

void EntityBoat::entityInit()
{
	dataWatcher->addObject(17, int_t(0));
	dataWatcher->addObject(18, int_t(1));
	dataWatcher->addObject(19, int_t(0));
}

int_t EntityBoat::getForwardDirection() const
{
	return dataWatcher->getWatchableObjectInt(18);
}

void EntityBoat::setForwardDirection(int_t value)
{
	dataWatcher->updateObject(18, value);
}

int_t EntityBoat::getTimeSinceHit() const
{
	return dataWatcher->getWatchableObjectInt(17);
}

void EntityBoat::setTimeSinceHit(int_t value)
{
	dataWatcher->updateObject(17, value);
}

int_t EntityBoat::getDamageTaken() const
{
	return dataWatcher->getWatchableObjectInt(19);
}

void EntityBoat::setDamageTaken(int_t value)
{
	dataWatcher->updateObject(19, value);
}

AxisAlignedBB *EntityBoat::getCollisionBox(Entity *entity)
{
	return entity->boundingBox;
}

bool EntityBoat::canBeCollidedWith()
{
	return !isDead;
}

bool EntityBoat::attackEntityFrom(Entity *entity, int_t i)
{
	if (!worldObj->multiplayerWorld && !isDead)
	{
		setForwardDirection(-getForwardDirection());
		setTimeSinceHit(10);
		setDamageTaken(JavaArithmetic::intAdd(getDamageTaken(), JavaArithmetic::intMul(i, 10)));
		setBeenAttacked();
		if (getDamageTaken() > 40)
		{
			if (riddenByEntity != nullptr)
			{
				riddenByEntity->mountEntity(this);
			}
			for (int j = 0; j < 3; j++)
			{
				dropItemWithOffset(Block::planks->blockID, 1, 0.0f);
			}
			for (int k = 0; k < 2; k++)
			{
				dropItemWithOffset(Item::stick->shiftedIndex, 1, 0.0f);
			}
			setEntityDead();
		}
		return true;
	}
	return true;
}

bool EntityBoat::attackEntityFrom(const DamageSource &source, int_t damage)
{
	return attackEntityFrom(source.getEntity(), damage);
}

void EntityBoat::performHurtAnimation()
{
	setForwardDirection(-getForwardDirection());
	setTimeSinceHit(10);
	setDamageTaken(JavaArithmetic::intMul(getDamageTaken(), 11));
}

void EntityBoat::setPositionAndRotation2(double d, double d1, double d2, float f, float f1, int_t i)
{
	boatX = d;
	boatY = d1;
	boatZ = d2;
	boatYaw = (double)f;
	boatPitch = (double)f1;
	boatTicks = i + 4;
	motionX = velocityX;
	motionY = velocityY;
	motionZ = velocityZ;
}

void EntityBoat::setVelocity(double d, double d1, double d2)
{
	velocityX = motionX = d;
	velocityY = motionY = d1;
	velocityZ = motionZ = d2;
}

void EntityBoat::onUpdate()
{
	Entity::onUpdate();
	if (getTimeSinceHit() > 0)
	{
		setTimeSinceHit(getTimeSinceHit() - 1);
	}
	if (getDamageTaken() > 0)
	{
		setDamageTaken(getDamageTaken() - 1);
	}
	prevPosX = posX;
	prevPosY = posY;
	prevPosZ = posZ;
	int i = 5;
	double d = 0.0;
	for (int j = 0; j < i; j++)
	{
		double d5 = (boundingBox->minY + ((boundingBox->maxY - boundingBox->minY) * (double)(j + 0)) / (double)i) - 0.125;
		double d9 = (boundingBox->minY + ((boundingBox->maxY - boundingBox->minY) * (double)(j + 1)) / (double)i) - 0.125;
		AxisAlignedBB *axisalignedbb = AxisAlignedBB::getBoundingBoxFromPool(boundingBox->minX, d5, boundingBox->minZ, boundingBox->maxX, d9, boundingBox->maxZ);
		if (worldObj->isAABBInMaterial(axisalignedbb, Material::water))
		{
			d += 1.0 / (double)i;
		}
	}
	const double horizontalSpeed = JavaMath::sqrt(motionX * motionX + motionZ * motionZ);
	if (horizontalSpeed > 0.15)
	{
#if PLATFORM_FLOAT_ENTITY_CORE_MATH
		const float directionRadians = rotationYaw * 3.1415927f / 180.0f;
		const float directionX = std::cos(directionRadians);
		const float directionZ = std::sin(directionRadians);
#else
		double directionX = JavaMath::cos(((double)rotationYaw * 3.1415926535897931) / 180.0);
		double directionZ = JavaMath::sin(((double)rotationYaw * 3.1415926535897931) / 180.0);
#endif
		for (int_t particle = 0; (double)particle < 1.0 + horizontalSpeed * 60.0; ++particle)
		{
			double randomSide = (double)(rand.nextFloat() * 2.0f - 1.0f);
			double sideOffset = (double)(rand.nextInt(2) * 2 - 1) * 0.7;
			if (rand.nextBoolean())
			{
				double particleX = posX - directionX * randomSide * 0.8 + directionZ * sideOffset;
				double particleZ = posZ - directionZ * randomSide * 0.8 - directionX * sideOffset;
				worldObj->spawnParticle("splash", particleX, posY - 0.125, particleZ, motionX, motionY, motionZ);
			}
			else
			{
				double particleX = posX + directionX + directionZ * randomSide * 0.7;
				double particleZ = posZ + directionZ - directionX * randomSide * 0.7;
				worldObj->spawnParticle("splash", particleX, posY - 0.125, particleZ, motionX, motionY, motionZ);
			}
		}
	}

	if (worldObj->multiplayerWorld)
	{
		if (boatTicks > 0)
		{
			double d1 = posX + (boatX - posX) / (double)boatTicks;
			double d2 = posY + (boatY - posY) / (double)boatTicks;
			double d3 = posZ + (boatZ - posZ) / (double)boatTicks;
			double d4;
			for (d4 = boatYaw - (double)rotationYaw; d4 < -180.0; d4 += 360.0) {}
			for (; d4 >= 180.0; d4 -= 360.0) {}
			rotationYaw = (float)((double)rotationYaw + d4 / (double)boatTicks);
			rotationPitch = (float)((double)rotationPitch + (boatPitch - (double)rotationPitch) / (double)boatTicks);
			boatTicks--;
			setPosition(d1, d2, d3);
			setRotation(rotationYaw, rotationPitch);
		}
		else
		{
			double d1 = posX + motionX;
			double d2 = posY + motionY;
			double d3 = posZ + motionZ;
			setPosition(d1, d2, d3);
			if (onGround)
			{
				motionX *= 0.5;
				motionY *= 0.5;
				motionZ *= 0.5;
			}
			motionX *= 0.99000000953674316;
			motionY *= 0.94999998807907104;
			motionZ *= 0.99000000953674316;
		}
		return;
	}
	if (d < 1.0)
	{
		double d1 = d * 2.0 - 1.0;
		motionY += 0.039999999105930328 * d1;
	}
	else
	{
		if (motionY < 0.0)
		{
			motionY /= 2.0;
		}
		motionY += 0.0070000002160668373;
	}
	if (riddenByEntity != nullptr)
	{
		motionX += riddenByEntity->motionX * 0.2;
		motionZ += riddenByEntity->motionZ * 0.2;
	}
	double maxSpeed = 0.40000000000000002;
	if (motionX < -maxSpeed) motionX = -maxSpeed;
	if (motionX >  maxSpeed) motionX =  maxSpeed;
	if (motionZ < -maxSpeed) motionZ = -maxSpeed;
	if (motionZ >  maxSpeed) motionZ =  maxSpeed;
	if (onGround)
	{
		motionX *= 0.5;
		motionY *= 0.5;
		motionZ *= 0.5;
	}
	moveEntity(motionX, motionY, motionZ);
	if (isCollidedHorizontally && horizontalSpeed > 0.2)
	{
		if (!worldObj->multiplayerWorld)
		{
			setEntityDead();
			for (int i1 = 0; i1 < 3; i1++)
			{
				dropItemWithOffset(Block::planks->blockID, 1, 0.0f);
			}
			for (int j1 = 0; j1 < 2; j1++)
			{
				dropItemWithOffset(Item::stick->shiftedIndex, 1, 0.0f);
			}
		}
	}
	else
	{
		motionX *= 0.99000000953674316;
		motionY *= 0.94999998807907104;
		motionZ *= 0.99000000953674316;
	}
	rotationPitch = 0.0f;
#if PLATFORM_FLOAT_ENTITY_CORE_MATH
	float d4 = rotationYaw;
	const float d7 = static_cast<float>(prevPosX - posX);
	const float d11 = static_cast<float>(prevPosZ - posZ);
	if (d7 * d7 + d11 * d11 > 0.001f)
	{
		d4 = (std::atan2(d11, d7) * 180.0f) / 3.1415927f;
	}
	float d13;
	for (d13 = d4 - rotationYaw; d13 >= 180.0f; d13 -= 360.0f) {}
	for (; d13 < -180.0f; d13 += 360.0f) {}
	if (d13 > 20.0f)
	{
		d13 = 20.0f;
	}
	if (d13 < -20.0f)
	{
		d13 = -20.0f;
	}
	rotationYaw += d13;
#else
	double d4 = rotationYaw;
	double d7 = prevPosX - posX;
	double d11 = prevPosZ - posZ;
	if (d7 * d7 + d11 * d11 > 0.001)
	{
		d4 = (float)((JavaMath::atan2(d11, d7) * 180.0) / 3.1415926535897931);
	}
	double d13;
	for (d13 = d4 - (double)rotationYaw; d13 >= 180.0; d13 -= 360.0) {}
	for (; d13 < -180.0; d13 += 360.0) {}
	if (d13 > 20.0)
	{
		d13 = 20.0;
	}
	if (d13 < -20.0)
	{
		d13 = -20.0;
	}
	rotationYaw = (float)((double)rotationYaw + d13);
#endif
	setRotation(rotationYaw, rotationPitch);
	const auto& list = worldObj->getEntitiesWithinAABBExcludingEntity(this, boundingBox->expand(0.20000000298023224, 0.0, 0.20000000298023224));
	if (list.size() > 0)
	{
		for (size_t j2 = 0; j2 < list.size(); j2++)
		{
			Entity *entity = list[j2];
			if (entity != riddenByEntity && entity->canBePushed() && dynamic_cast<EntityBoat*>(entity) != nullptr)
			{
				entity->applyEntityCollision(this);
			}
		}
	}
	for (int k1 = 0; k1 < 4; k1++)
	{
		int l1 = MathHelper::floor_double(posX + ((double)(k1 % 2) - 0.5) * 0.80000000000000004);
		int i2 = MathHelper::floor_double(posY);
		int j2 = MathHelper::floor_double(posZ + ((double)(k1 / 2) - 0.5) * 0.80000000000000004);
		if (worldObj->getBlockId(l1, i2, j2) == Block::snow->blockID)
		{
			worldObj->setBlockWithNotify(l1, i2, j2, 0);
		}
	}
	if (riddenByEntity != nullptr && riddenByEntity->isDead)
	{
		riddenByEntity = nullptr;
	}
}

void EntityBoat::writeEntityToNBT(NBTTagCompound *nbttagcompound)
{
}

void EntityBoat::readEntityFromNBT(NBTTagCompound *nbttagcompound)
{
}

double EntityBoat::getMountedYOffset()
{
	return (double)height * 0.0 - 0.30000001192092896;
}

bool EntityBoat::interact(EntityPlayer *entityplayer)
{
	if (riddenByEntity != nullptr && riddenByEntity->isPlayer() && riddenByEntity != entityplayer)
	{
		return true;
	}
	if (!worldObj->multiplayerWorld)
	{
		entityplayer->mountEntity(this);
	}
	return true;
}

void EntityBoat::updateRiderPosition()
{
	if (riddenByEntity == nullptr)
	{
		return;
	}
	double d = JavaMath::cos(((double)rotationYaw * 3.1415926535897931) / 180.0) * 0.4;
	double d1 = JavaMath::sin(((double)rotationYaw * 3.1415926535897931) / 180.0) * 0.4;
	riddenByEntity->setPosition(posX + d, posY + getMountedYOffset() + riddenByEntity->getYOffset(), posZ + d1);
}

float EntityBoat::getShadowSize()
{
	return 0.0f;
}
