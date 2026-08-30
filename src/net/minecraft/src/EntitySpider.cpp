#include "EntitySpider.h"

#include <cmath>

#include "EntityPlayer.h"
#include "DataWatcher.h"
#include "Potion.h"
#include "PotionEffect.h"
#include "Item.h"
#include "MathHelper.h"
#include "NBTTagCompound.h"
#include "World.h"
#include "platform/PlatformTuning.h"

EntitySpider::EntitySpider(World *world)
	: EntityMob(world)
{
	ensureEntityInit();
	texture = "/mob/spider.png";
	setSize(1.4f, 0.9f);
	moveSpeed = 0.8f;
	setHealth(getMaxHealth());
}

void EntitySpider::entityInit()
{
	EntityMob::entityInit();
	if (!dataWatcher->hasObject(16))
		dataWatcher->addObject(16, (byte_t)0);
}

void EntitySpider::onUpdate()
{
	EntityMob::onUpdate();
	if (!worldObj->multiplayerWorld)
		setBesideClimbableBlock(isCollidedHorizontally);
}

int_t EntitySpider::getMaxHealth() const
{
	return 16;
}

double EntitySpider::getMountedYOffset()
{
	return (double)height * 0.75 - 0.5;
}

bool EntitySpider::canTriggerWalking()
{
	return false;
}

Entity *EntitySpider::findPlayerToAttack()
{
	float f = getEntityBrightness(1.0f);
	if (f < 0.5f)
	{
		double d = 16.0;
		return worldObj->getClosestVulnerablePlayerToEntity(this, d);
	}
	return nullptr;
}

jstring EntitySpider::getLivingSound() { return "mob.spider"; }
jstring EntitySpider::getHurtSound()   { return "mob.spider"; }
jstring EntitySpider::getDeathSound()  { return "mob.spiderdeath"; }

void EntitySpider::attackEntity(Entity *entity, float f)
{
	float f1 = getEntityBrightness(1.0f);
	if (f1 > 0.5f && rand.nextInt(100) == 0)
	{
		playerToAttack = nullptr;
		return;
	}
	if (f > 2.0f && f < 6.0f && rand.nextInt(10) == 0)
	{
		if (onGround)
		{
#if PLATFORM_FLOAT_ENTITY_AI_MATH
			const float d = (float)(entity->posX - posX);
			const float d1 = (float)(entity->posZ - posZ);
			const float f2 = MathHelper::sqrt_float(d * d + d1 * d1);
			if (f2 > 0.0f)
			{
				motionX = (double)((d / f2) * 0.4f) + motionX * 0.20000000298023224;
				motionZ = (double)((d1 / f2) * 0.4f) + motionZ * 0.20000000298023224;
			}
#else
			double d = entity->posX - posX;
			double d1 = entity->posZ - posZ;
			float f2 = MathHelper::sqrt_double(d * d + d1 * d1);
			motionX = (d / (double)f2) * 0.5 * 0.80000001192092896 + motionX * 0.20000000298023224;
			motionZ = (d1 / (double)f2) * 0.5 * 0.80000001192092896 + motionZ * 0.20000000298023224;
#endif
			motionY = 0.40000000596046448;
		}
	}
	else
	{
		EntityMob::attackEntity(entity, f);
	}
}

void EntitySpider::writeEntityToNBT(NBTTagCompound *nbttagcompound)
{
	EntityMob::writeEntityToNBT(nbttagcompound);
}

void EntitySpider::readEntityFromNBT(NBTTagCompound *nbttagcompound)
{
	EntityMob::readEntityFromNBT(nbttagcompound);
}

int_t EntitySpider::getDropItemId()
{
	return Item::silk->shiftedIndex;
}

void EntitySpider::dropFewItems(bool recentlyHitByPlayer, int_t lootingLevel)
{
	EntityMob::dropFewItems(recentlyHitByPlayer, lootingLevel);
	if (recentlyHitByPlayer && Item::spiderEye != nullptr &&
		(rand.nextInt(3) == 0 || rand.nextInt(1 + lootingLevel) > 0))
	{
		dropItem(Item::spiderEye->shiftedIndex, 1);
	}
}

bool EntitySpider::isOnLadder()
{
	return isBesideClimbableBlock();
}

void EntitySpider::setInWeb()
{
}

bool EntitySpider::isPotionApplicable(PotionEffect *effect) const
{
	if (effect != nullptr && Potion::poison != nullptr && effect->getPotionID() == Potion::poison->id)
		return false;
	return EntityMob::isPotionApplicable(effect);
}


float EntitySpider::spiderScaleAmount()
{
	return 1.0f;
}

bool EntitySpider::isBesideClimbableBlock() const
{
	return (dataWatcher->getWatchableObjectByte(16) & 1) != 0;
}

void EntitySpider::setBesideClimbableBlock(bool climbing)
{
	byte_t value = dataWatcher->getWatchableObjectByte(16);
	dataWatcher->updateObject(16, climbing ? (byte_t)(value | 1) : (byte_t)(value & 0xfe));
}

EnumCreatureAttribute EntitySpider::getCreatureAttribute() const
{
	return EnumCreatureAttribute::ARTHROPOD;
}
