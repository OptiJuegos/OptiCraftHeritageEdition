#include "EntityBlaze.h"

#include "java/Arithmetic.h"

#include <cmath>

#include "AxisAlignedBB.h"
#include "DataWatcher.h"
#include "DamageSource.h"
#include "EntityPlayer.h"
#include "EntitySmallFireball.h"
#include "Item.h"
#include "MathHelper.h"
#include "java/Math.h"
#include "NBTTagCompound.h"
#include "World.h"

EntityBlaze::EntityBlaze(World *world)
	: EntityMob(world), heightOffset(0.5f), heightOffsetUpdateTime(0), attackStep(0)
{
	ensureEntityInit();
	texture = "/mob/fire.png";
	immuneToFire = true;
	attackStrength = 6;
	experienceValue = 10;
}

int_t EntityBlaze::getMaxHealth() const
{
	return 20;
}

void EntityBlaze::entityInit()
{
	EntityMob::entityInit();
	if (!dataWatcher->hasObject(16))
		dataWatcher->addObject(16, static_cast<byte_t>(0));
}

jstring EntityBlaze::getLivingSound()
{
	return "mob.blaze.breathe";
}

jstring EntityBlaze::getHurtSound()
{
	return "mob.blaze.hit";
}

jstring EntityBlaze::getDeathSound()
{
	return "mob.blaze.death";
}

int_t EntityBlaze::getBrightnessForRender(float)
{
	return 15728880;
}

float EntityBlaze::getEntityBrightness(float)
{
	return 1.0f;
}

bool EntityBlaze::attackEntityFrom(const DamageSource &source, int_t damage)
{
	return EntityMob::attackEntityFrom(source, damage);
}

void EntityBlaze::onLivingUpdate()
{
	if (!worldObj->multiplayerWorld)
	{
		if (isWet())
			attackEntityFrom(DamageSource::drown, 1);

		--heightOffsetUpdateTime;
		if (heightOffsetUpdateTime <= 0)
		{
			heightOffsetUpdateTime = 100;
			heightOffset = 0.5f + static_cast<float>(rand.nextGaussian()) * 3.0f;
		}

		Entity *target = getTarget();
		if (target != nullptr && target->posY + static_cast<double>(target->getEyeHeight()) >
			posY + static_cast<double>(getEyeHeight()) + static_cast<double>(heightOffset))
		{
			motionY += (0.30000001192092896 - motionY) * 0.30000001192092896;
		}
	}

	if (rand.nextInt(24) == 0)
	{
		const float soundVolume = 1.0f + rand.nextFloat();
		const float soundPitch = rand.nextFloat() * 0.7f + 0.3f;
		worldObj->playSoundEffect(posX + 0.5, posY + 0.5, posZ + 0.5, "fire.fire", soundVolume, soundPitch);
	}

	if (!onGround && motionY < 0.0)
		motionY *= 0.6;

	for (int_t i = 0; i < 2; ++i)
	{
		const double particleX = posX + (rand.nextDouble() - 0.5) * static_cast<double>(width);
		const double particleY = posY + rand.nextDouble() * static_cast<double>(height);
		const double particleZ = posZ + (rand.nextDouble() - 0.5) * static_cast<double>(width);
		worldObj->spawnParticle("largesmoke", particleX, particleY, particleZ, 0.0, 0.0, 0.0);
	}

	EntityMob::onLivingUpdate();
}

void EntityBlaze::attackEntity(Entity *entity, float distance)
{
	if (entity == nullptr)
		return;

	if (attackTime <= 0 && distance < 2.0f && entity->boundingBox->maxY > boundingBox->minY &&
		entity->boundingBox->minY < boundingBox->maxY)
	{
		attackTime = 20;
		attackEntityAsMob(entity);
	}
	else if (distance < 30.0f)
	{
		const double dx = entity->posX - posX;
		const double dy = entity->boundingBox->minY + static_cast<double>(entity->height / 2.0f) -
			(posY + static_cast<double>(height / 2.0f));
		const double dz = entity->posZ - posZ;
		if (attackTime == 0)
		{
			++attackStep;
			if (attackStep == 1)
			{
				attackTime = 60;
				setBlazing(true);
			}
			else if (attackStep <= 4)
			{
				attackTime = 6;
			}
			else
			{
				attackTime = 100;
				attackStep = 0;
				setBlazing(false);
			}

			if (attackStep > 1)
			{
				const float spread = MathHelper::sqrt_float(distance) * 0.5f;
				worldObj->playAuxSFXAtEntity(nullptr, 1009, JavaArithmetic::doubleToInt(posX), JavaArithmetic::doubleToInt(posY),
					JavaArithmetic::doubleToInt(posZ), 0);
				const double fireballX = dx + rand.nextGaussian() * static_cast<double>(spread);
				const double fireballZ = dz + rand.nextGaussian() * static_cast<double>(spread);
				EntitySmallFireball *fireball = new EntitySmallFireball(worldObj, this, fireballX, dy, fireballZ);
				fireball->posY = posY + static_cast<double>(height / 2.0f) + 0.5;
				if (!worldObj->entityJoinedWorld(fireball))
					delete fireball;
			}
		}

		rotationYaw = static_cast<float>(JavaMath::atan2(dz, dx) * 180.0 / 3.1415927410125732) - 90.0f;
		hasAttacked = true;
	}
}

void EntityBlaze::fall(float)
{
}

void EntityBlaze::writeEntityToNBT(NBTTagCompound *tag)
{
	EntityMob::writeEntityToNBT(tag);
}

void EntityBlaze::readEntityFromNBT(NBTTagCompound *tag)
{
	EntityMob::readEntityFromNBT(tag);
}

int_t EntityBlaze::getDropItemId()
{
	return Item::blazeRod != nullptr ? Item::blazeRod->shiftedIndex : 0;
}

bool EntityBlaze::isBurning()
{
	return isBlazing();
}

void EntityBlaze::dropFewItems(bool recentlyHitByPlayer, int_t lootingLevel)
{
	if (!recentlyHitByPlayer || Item::blazeRod == nullptr)
		return;
	const int_t count = rand.nextInt(2 + lootingLevel);
	for (int_t i = 0; i < count; ++i)
		dropItem(Item::blazeRod->shiftedIndex, 1);
}

bool EntityBlaze::isBlazing() const
{
	return (const_cast<EntityBlaze *>(this)->dataWatcher->getWatchableObjectByte(16) & 1) != 0;
}

void EntityBlaze::setBlazing(bool blazing)
{
	byte_t value = dataWatcher->getWatchableObjectByte(16);
	if (blazing)
		value = static_cast<byte_t>(value | 1);
	else
		value = static_cast<byte_t>(value & static_cast<byte_t>(~1));
	dataWatcher->updateObject(16, value);
}

bool EntityBlaze::isValidLightLevel()
{
	return true;
}
