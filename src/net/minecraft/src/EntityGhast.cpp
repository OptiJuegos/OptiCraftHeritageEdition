#include "EntityGhast.h"
#include "java/Arithmetic.h"

#include <cmath>
#include "java/Math.h"

#include "AchievementList.h"
#include "AxisAlignedBB.h"
#include "DamageSource.h"
#include "EntityFireball.h"
#include "EntityPlayer.h"
#include "Item.h"
#include "MathHelper.h"
#include "Vec3D.h"
#include "World.h"
#include "DataWatcher.h"
#include "platform/PlatformTuning.h"

EntityGhast::EntityGhast(World *world)
	: EntityFlying(world)
{
	ensureEntityInit();
	courseChangeCooldown = 0;
	targetedEntityId = -1;
	aggroCooldown = 0;
	prevAttackCounter = 0;
	attackCounter = 0;
	texture = "/mob/ghast.png";
	setSize(4.0f, 4.0f);
	immuneToFire = true;
	experienceValue = 5;
	setHealth(getMaxHealth());
}

void EntityGhast::entityInit()
{
	EntityFlying::entityInit();
	dataWatcher->addObject(16, (byte_t)0);
}

int_t EntityGhast::getMaxHealth() const
{
	return 10;
}

bool EntityGhast::attackEntityFrom(const DamageSource &source, int_t damage)
{
	if (source.getDamageType() == "fireball")
	{
		EntityPlayer *player = dynamic_cast<EntityPlayer *>(source.getEntity());
		if (player != nullptr)
		{
			EntityFlying::attackEntityFrom(source, 1000);
			if (AchievementList::ghast != nullptr)
				player->triggerAchievement(AchievementList::ghast);
			return true;
		}
	}
	return EntityFlying::attackEntityFrom(source, damage);
}

void EntityGhast::onUpdate()
{
	EntityFlying::onUpdate();
	byte_t byte0 = dataWatcher->getWatchableObjectByte(16);
	texture = byte0 != 1 ? "/mob/ghast.png" : "/mob/ghast_fire.png";
}

void EntityGhast::updatePlayerActionState()
{
	if (!worldObj->multiplayerWorld && worldObj->difficultySetting == 0)
	{
		setEntityDead();
	}
	despawnEntity();
	prevAttackCounter = attackCounter;
	double d = waypointX - posX;
	double d1 = waypointY - posY;
	double d2 = waypointZ - posZ;
	double d3 = MathHelper::sqrt_double(d * d + d1 * d1 + d2 * d2);
	if (d3 < 1.0 || d3 > 60.0)
	{
		waypointX = posX + (double)((rand.nextFloat() * 2.0f - 1.0f) * 16.0f);
		waypointY = posY + (double)((rand.nextFloat() * 2.0f - 1.0f) * 16.0f);
		waypointZ = posZ + (double)((rand.nextFloat() * 2.0f - 1.0f) * 16.0f);
	}
	if (courseChangeCooldown-- <= 0)
	{
		courseChangeCooldown += rand.nextInt(5) + 2;
		if (isCourseTraversable(waypointX, waypointY, waypointZ, d3))
		{
			motionX += (d / d3) * 0.10000000000000001;
			motionY += (d1 / d3) * 0.10000000000000001;
			motionZ += (d2 / d3) * 0.10000000000000001;
		}
		else
		{
			waypointX = posX;
			waypointY = posY;
			waypointZ = posZ;
		}
	}
	Entity *targetedEntity = targetedEntityId >= 0 ? worldObj->getEntityByID(targetedEntityId) : nullptr;
	if (targetedEntity != nullptr && targetedEntity->isDead)
		targetedEntity = nullptr;

	if (targetedEntity == nullptr || aggroCooldown-- <= 0)
	{
		targetedEntity = worldObj->getClosestVulnerablePlayerToEntity(this, 100.0);
		if (targetedEntity != nullptr)
			aggroCooldown = 20;
	}
	targetedEntityId = targetedEntity != nullptr ? targetedEntity->entityId : -1;
	double d4 = 64.0;
	if (targetedEntity != nullptr && targetedEntity->getDistanceSqToEntity(this) < d4 * d4)
	{
		double d5 = targetedEntity->posX - posX;
		double d6 = (targetedEntity->boundingBox->minY + (double)(targetedEntity->height / 2.0f)) - (posY + (double)(height / 2.0f));
		double d7 = targetedEntity->posZ - posZ;
#if PLATFORM_FLOAT_ENTITY_AI_MATH
		renderYawOffset = rotationYaw = -std::atan2(static_cast<float>(d5), static_cast<float>(d7)) * 180.0f / 3.1415927f;
#else
		renderYawOffset = rotationYaw = -(float)JavaMath::atan2(d5, d7) * 180.0f / 3.1415927f;
#endif
		if (canEntityBeSeen(targetedEntity))
		{
			if (attackCounter == 10)
			{
				worldObj->playAuxSFXAtEntity(nullptr, 1007, JavaArithmetic::doubleToInt(posX), JavaArithmetic::doubleToInt(posY), JavaArithmetic::doubleToInt(posZ), 0);
			}
			attackCounter++;
			if (attackCounter == 20)
			{
				worldObj->playAuxSFXAtEntity(nullptr, 1008, JavaArithmetic::doubleToInt(posX), JavaArithmetic::doubleToInt(posY), JavaArithmetic::doubleToInt(posZ), 0);
				EntityFireball *entityfireball = new EntityFireball(worldObj, this, d5, d6, d7);
				double d8 = 4.0;
				Vec3D *vec3d = getLook(1.0f);
				entityfireball->posX = posX + vec3d->xCoord * d8;
				entityfireball->posY = posY + (double)(height / 2.0f) + 0.5;
				entityfireball->posZ = posZ + vec3d->zCoord * d8;
				if (!worldObj->entityJoinedWorld(entityfireball))
					delete entityfireball;
				attackCounter = -40;
			}
		}
		else if (attackCounter > 0)
		{
			attackCounter--;
		}
	}
	else
	{
#if PLATFORM_FLOAT_ENTITY_AI_MATH
		renderYawOffset = rotationYaw = -std::atan2(static_cast<float>(motionX), static_cast<float>(motionZ)) * 180.0f / 3.1415927f;
#else
		renderYawOffset = rotationYaw = -(float)JavaMath::atan2(motionX, motionZ) * 180.0f / 3.1415927f;
#endif
		if (attackCounter > 0)
		{
			attackCounter--;
		}
	}
	if (!worldObj->multiplayerWorld)
	{
		byte_t byte0 = dataWatcher->getWatchableObjectByte(16);
		byte_t byte1 = (byte_t)(attackCounter <= 10 ? 0 : 1);
		if (byte0 != byte1)
		{
			dataWatcher->updateObject(16, byte1);
		}
	}
}

bool EntityGhast::isCourseTraversable(double d, double d1, double d2, double d3)
{
	double d4 = (waypointX - posX) / d3;
	double d5 = (waypointY - posY) / d3;
	double d6 = (waypointZ - posZ) / d3;
	AxisAlignedBB *axisalignedbb = boundingBox->copy();
	for (int i = 1; (double)i < d3; i++)
	{
		axisalignedbb->offset(d4, d5, d6);
		if (worldObj->getCollidingBoundingBoxes(this, axisalignedbb).size() > 0)
		{
			return false;
		}
	}
	return true;
}

jstring EntityGhast::getLivingSound() { return "mob.ghast.moan"; }
jstring EntityGhast::getHurtSound()   { return "mob.ghast.scream"; }
jstring EntityGhast::getDeathSound()  { return "mob.ghast.death"; }

int_t EntityGhast::getDropItemId()
{
	return Item::gunpowder->shiftedIndex;
}

void EntityGhast::dropFewItems(bool, int_t lootingLevel)
{
	const int_t tearCountBase = rand.nextInt(2);
	const int_t tearCountLooting = rand.nextInt(1 + lootingLevel);
	const int_t tearCount = tearCountBase + tearCountLooting;
	if (Item::ghastTear != nullptr)
	{
		for (int_t i = 0; i < tearCount; ++i)
			dropItem(Item::ghastTear->shiftedIndex, 1);
	}

	const int_t powderCountBase = rand.nextInt(3);
	const int_t powderCountLooting = rand.nextInt(1 + lootingLevel);
	const int_t powderCount = powderCountBase + powderCountLooting;
	for (int_t i = 0; i < powderCount; ++i)
		dropItem(Item::gunpowder->shiftedIndex, 1);
}

float EntityGhast::getSoundVolume()
{
	return 10.0f;
}

bool EntityGhast::getCanSpawnHere()
{
	return rand.nextInt(20) == 0 && EntityFlying::getCanSpawnHere() && worldObj->difficultySetting > 0;
}

int_t EntityGhast::getMaxSpawnedInChunk()
{
	return 1;
}
