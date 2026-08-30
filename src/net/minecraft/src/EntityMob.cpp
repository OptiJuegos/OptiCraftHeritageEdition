#include "EntityMob.h"
#include "java/Arithmetic.h"

#include "AxisAlignedBB.h"
#include "DamageSource.h"
#include "EntityPlayer.h"
#include "EnumSkyBlock.h"
#include "MathHelper.h"
#include "Potion.h"
#include "PotionEffect.h"
#include "World.h"

EntityMob::EntityMob(World *world)
	: EntityCreature(world), attackStrength(2)
{
	experienceValue = 5;
}

void EntityMob::onLivingUpdate()
{
	if (getEntityBrightness(1.0f) > 0.5f)
		entityAge += 2;
	EntityCreature::onLivingUpdate();
}

void EntityMob::onUpdate()
{
	EntityCreature::onUpdate();
	if (!worldObj->multiplayerWorld && worldObj->difficultySetting == 0)
		setEntityDead();
}

Entity *EntityMob::findPlayerToAttack()
{
	EntityPlayer *player = worldObj->getClosestVulnerablePlayerToEntity(this, 16.0);
	return player != nullptr && canEntityBeSeen(player) ? player : nullptr;
}

bool EntityMob::attackEntityFrom(Entity *entity, int_t damage)
{
	if (!EntityCreature::attackEntityFrom(entity, damage))
		return false;
	if (riddenByEntity != entity && ridingEntity != entity && entity != this)
		playerToAttack = entity;
	return true;
}

bool EntityMob::attackEntityFrom(const DamageSource &source, int_t damage)
{
	if (!EntityCreature::attackEntityFrom(source, damage))
		return false;
	Entity *entity = source.getEntity();
	if (riddenByEntity != entity && ridingEntity != entity && entity != this)
		playerToAttack = entity;
	return true;
}

bool EntityMob::attackEntityAsMob(Entity *target)
{
	if (target == nullptr)
		return false;

	Potion::initPotions();
	int_t damage = attackStrength;
	if (isPotionActive(Potion::damageBoost))
	{
		PotionEffect *effect = getActivePotionEffect(Potion::damageBoost);
		if (effect != nullptr)
			damage += JavaArithmetic::intShl(3, effect->getAmplifier());
	}
	if (isPotionActive(Potion::weakness))
	{
		PotionEffect *effect = getActivePotionEffect(Potion::weakness);
		if (effect != nullptr)
			damage -= JavaArithmetic::intShl(2, effect->getAmplifier());
	}

	return target->attackEntityFrom(DamageSource::causeMobDamage(this), damage);
}

void EntityMob::attackEntity(Entity *entity, float distance)
{
	if (entity != nullptr && attackTime <= 0 && distance < 2.0f && entity->boundingBox->maxY > boundingBox->minY &&
		entity->boundingBox->minY < boundingBox->maxY)
	{
		attackTime = 20;
		attackEntityAsMob(entity);
	}
}

float EntityMob::getBlockPathWeight(int_t x, int_t y, int_t z)
{
	return 0.5f - worldObj->getLightBrightness(x, y, z);
}

bool EntityMob::isValidLightLevel()
{
	const int_t x = MathHelper::floor_double(posX);
	const int_t y = MathHelper::floor_double(boundingBox->minY);
	const int_t z = MathHelper::floor_double(posZ);
	if (worldObj->getSavedLightValue(EnumSkyBlock::Sky, x, y, z) > rand.nextInt(32))
		return false;

	int_t light = worldObj->getBlockLightValue(x, y, z);
	if (worldObj->isThundering())
	{
		const int_t previous = worldObj->skylightSubtracted;
		worldObj->skylightSubtracted = 10;
		light = worldObj->getBlockLightValue(x, y, z);
		worldObj->skylightSubtracted = previous;
	}
	return light <= rand.nextInt(8);
}

bool EntityMob::getCanSpawnHere()
{
	return isValidLightLevel() && EntityCreature::getCanSpawnHere();
}
