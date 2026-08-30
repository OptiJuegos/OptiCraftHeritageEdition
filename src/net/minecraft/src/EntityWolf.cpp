#include "EntityWolf.h"

#include <cmath>

#include "EntityArrow.h"
#include "EntityAIAttackOnCollide.h"
#include "EntityAIBeg.h"
#include "EntityAIFollowOwner.h"
#include "EntityAIHurtByTarget.h"
#include "EntityAILeapAtTarget.h"
#include "EntityAILookIdle.h"
#include "EntityAIMate.h"
#include "EntityAIOwnerHurtByTarget.h"
#include "EntityAIOwnerHurtTarget.h"
#include "EntityAISwimming.h"
#include "EntityAISit.h"
#include "EntityAITargetNonTamed.h"
#include "EntityAIWander.h"
#include "EntityAIWatchClosest.h"
#include "EntityLiving.h"
#include "EntitySheep.h"

#include "EntityPlayer.h"
#include "InventoryPlayer.h"
#include "Item.h"
#include "ItemFood.h"
#include "ItemStack.h"
#include "MathHelper.h"
#include "NBTTagCompound.h"
#include "PathNavigate.h"
#include "World.h"
#include "AxisAlignedBB.h"
#include "DataWatcher.h"
#include "DamageSource.h"
#include "java/String.h"
#include "platform/PlatformTuning.h"


EntityWolf::EntityWolf(World *world)
	: EntityTameable(world)
{
	ensureEntityInit();
	looksWithInterest = false;
	field_25048_b = 0.0f;
	field_25054_c = 0.0f;
	isWolfShaking = false;
	field_25052_g = false;
	timeWolfIsShaking = 0.0f;
	prevTimeWolfIsShaking = 0.0f;
	texture = "/mob/wolf.png";
	setSize(0.6f, 0.8f);
	moveSpeed = 0.3f;
	setHealth(getMaxHealth());
	if (getNavigator() != nullptr)
		getNavigator()->setAvoidsWater(true);
	tasks.addTask(1, new EntityAISwimming(this));
	tasks.addTask(2, aiSit);
	tasks.addTask(3, new EntityAILeapAtTarget(this, 0.4f));
	tasks.addTask(4, new EntityAIAttackOnCollide(this, moveSpeed, true));
	tasks.addTask(5, new EntityAIFollowOwner(this, moveSpeed, 10.0f, 2.0f));
	tasks.addTask(6, new EntityAIMate(this, moveSpeed));
	tasks.addTask(7, new EntityAIWander(this, moveSpeed));
	tasks.addTask(8, new EntityAIBeg(this, 8.0f));
	tasks.addTask(9, new EntityAIWatchClosest(this, typeid(EntityPlayer), 8.0f));
	tasks.addTask(9, new EntityAILookIdle(this));
	targetTasks.addTask(1, new EntityAIOwnerHurtByTarget(this));
	targetTasks.addTask(2, new EntityAIOwnerHurtTarget(this));
	targetTasks.addTask(3, new EntityAIHurtByTarget(this, true));
	targetTasks.addTask(4, new EntityAITargetNonTamed(this, typeid(EntitySheep), 16.0f, 200, false));
}

bool EntityWolf::isAIEnabled()
{
	return true;
}

int_t EntityWolf::getMaxHealth() const
{
	return isTamed() ? 20 : 8;
}

void EntityWolf::setAttackTarget(EntityLiving *target)
{
	EntityTameable::setAttackTarget(target);
	if (dynamic_cast<EntityPlayer *>(target) != nullptr)
		setWolfAngry(true);
}

void EntityWolf::updateAITick()
{
	dataWatcher->updateObject(18, (int_t)health);
}

bool EntityWolf::attackEntityAsMob(Entity *target)
{
	if (target == nullptr)
		return false;
	DamageSource source = DamageSource::causeMobDamage(this);
	return target->attackEntityFrom(source, isTamed() ? 4 : 2);
}

bool EntityWolf::isWheat(ItemStack *itemstack) const
{
	if (itemstack == nullptr)
		return false;
	ItemFood *food = dynamic_cast<ItemFood *>(itemstack->getItem());
	return food != nullptr && food->getIsWolfsFavoriteMeat();
}

EntityAnimal *EntityWolf::spawnBabyAnimal(EntityAnimal *)
{
	EntityWolf *child = new EntityWolf(worldObj);
	child->setOwner(getOwnerName());
	child->setTamed(true);
	return child;
}

bool EntityWolf::canMateWith(EntityAnimal *mate) const
{
	if (mate == this || !isTamed())
		return false;
	EntityWolf *wolf = dynamic_cast<EntityWolf *>(mate);
	return wolf != nullptr && wolf->isTamed() && !wolf->isSitting() && isInLove() && wolf->isInLove();
}

void EntityWolf::setLooksWithInterest(bool value)
{
	looksWithInterest = value;
}

void EntityWolf::entityInit()
{
	EntityTameable::entityInit();
	dataWatcher->addObject(18, (int_t)0);
}

bool EntityWolf::canTriggerWalking()
{
	return false;
}

const char *EntityWolf::getEntityTexture()
{
	if (isWolfTamed())
		return "/mob/wolf_tame.png";
	if (isWolfAngry())
		return "/mob/wolf_angry.png";
	return "/mob/wolf.png";
}

void EntityWolf::writeEntityToNBT(NBTTagCompound *nbttagcompound)
{
	EntityTameable::writeEntityToNBT(nbttagcompound);
	nbttagcompound->setBoolean("Angry", isWolfAngry());
	nbttagcompound->setBoolean("Sitting", isWolfSitting());
	if (getWolfOwner().length() > 0)
	{
		nbttagcompound->setString("Owner", getWolfOwner());
	}
}

void EntityWolf::readEntityFromNBT(NBTTagCompound *nbttagcompound)
{
	EntityTameable::readEntityFromNBT(nbttagcompound);
	setWolfAngry(nbttagcompound->getBoolean("Angry"));
	setWolfSitting(nbttagcompound->getBoolean("Sitting"));
	jstring s = nbttagcompound->getString("Owner");
	if (s.length() > 0)
	{
		setWolfOwner(s);
		setWolfTamed(true);
	}
}

bool EntityWolf::canDespawn()
{
	return isWolfAngry();
}

jstring EntityWolf::getLivingSound()
{
	if (isWolfAngry())
		return "mob.wolf.growl";
	if (rand.nextInt(3) == 0)
	{
		if (isWolfTamed() && dataWatcher->getWatchableObjectInt(18) < 10)
			return "mob.wolf.whine";
		return "mob.wolf.panting";
	}
	return "mob.wolf.bark";
}
jstring EntityWolf::getHurtSound()   { return "mob.wolf.hurt"; }
jstring EntityWolf::getDeathSound()  { return "mob.wolf.death"; }

float EntityWolf::getSoundVolume()
{
	return 0.4f;
}

int_t EntityWolf::getDropItemId()
{
	return -1;
}

void EntityWolf::updatePlayerActionState()
{
	EntityTameable::updatePlayerActionState();
	if (!hasAttacked && !hasPath() && isWolfTamed() && ridingEntity == nullptr)
	{
		EntityPlayer *owner = worldObj->getPlayerEntityByName(getWolfOwner());
		if (owner != nullptr)
		{
#if PLATFORM_FLOAT_ENTITY_AI_MATH
			const float dx = (float)(owner->posX - posX);
			const float dy = (float)(owner->posY - posY);
			const float dz = (float)(owner->posZ - posZ);
			const float distanceSq = dx * dx + dy * dy + dz * dz;
			if (distanceSq > 25.0f)
				getPathOrWalkableBlock(owner, distanceSq);
#else
			float f = owner->getDistanceToEntity(this);
			if (f > 5.0f)
				getPathOrWalkableBlock(owner, f);
#endif
		}
		else if (!isInWater())
		{
			setWolfSitting(true);
		}
	}
	else if (playerToAttack == nullptr && !hasPath() && !isWolfTamed() && worldObj->rand.nextInt(100) == 0)
	{
		AxisAlignedBB *area = AxisAlignedBB::getBoundingBoxFromPool(posX, posY, posZ, posX + 1.0, posY + 1.0, posZ + 1.0)->expand(16.0, 4.0, 16.0);
		const auto &sheep = worldObj->getEntitiesWithinAABB(typeid(EntitySheep), area);
		if (!sheep.empty())
			setTarget(sheep[(size_t)worldObj->rand.nextInt((int_t)sheep.size())]);
	}
	if (isInWater())
		setWolfSitting(false);
	if (!worldObj->multiplayerWorld)
		dataWatcher->updateObject(18, (int_t)health);
}

void EntityWolf::onLivingUpdate()
{
	EntityAnimal::onLivingUpdate();
	looksWithInterest = false;
	if (hasCurrentTarget() && !hasPath() && !isWolfAngry())
	{
		Entity *entity = getCurrentTarget();
		if (entity != nullptr && entity->isPlayer())
		{
			EntityPlayer *player = static_cast<EntityPlayer *>(entity);
			ItemStack *itemstack = player->inventory->getCurrentItem();
			if (itemstack != nullptr)
			{
				if (!isWolfTamed() && itemstack->itemID == Item::bone->shiftedIndex)
					looksWithInterest = true;
				else if (isWolfTamed())
				{
					ItemFood *food = dynamic_cast<ItemFood *>(itemstack->getItem());
					if (food != nullptr)
						looksWithInterest = food->getIsWolfsFavoriteMeat();
				}
			}
		}
	}
	if (!worldObj->multiplayerWorld && isWolfShaking && !field_25052_g && !hasPath() && onGround)
	{
		field_25052_g = true;
		timeWolfIsShaking = 0.0f;
		prevTimeWolfIsShaking = 0.0f;
		worldObj->setEntityState(this, (byte_t)8);
	}
}

void EntityWolf::onUpdate()
{
	EntityAnimal::onUpdate();
	if (worldObj->multiplayerWorld)
		health = dataWatcher->getWatchableObjectInt(18);

	field_25054_c = field_25048_b;
	if (looksWithInterest)
		field_25048_b += (1.0f - field_25048_b) * 0.4f;
	else
		field_25048_b += (0.0f - field_25048_b) * 0.4f;
	if (looksWithInterest)
		numTicksToChaseTarget = 10;

	if (isWet())
	{
		isWolfShaking = true;
		field_25052_g = false;
		timeWolfIsShaking = 0.0f;
		prevTimeWolfIsShaking = 0.0f;
	}
	else if ((isWolfShaking || field_25052_g) && field_25052_g)
	{
		if (timeWolfIsShaking == 0.0f)
			worldObj->playSoundAtEntity(this, "mob.wolf.shake", getSoundVolume(), rand.nextFloatDifference() * 0.2f + 1.0f);
		prevTimeWolfIsShaking = timeWolfIsShaking;
		timeWolfIsShaking += 0.05f;
		if (prevTimeWolfIsShaking >= 2.0f)
		{
			isWolfShaking = false;
			field_25052_g = false;
			prevTimeWolfIsShaking = 0.0f;
			timeWolfIsShaking = 0.0f;
		}
		if (timeWolfIsShaking > 0.4f)
		{
			float y = (float)boundingBox->minY;
			int_t count = (int_t)(MathHelper::sin((timeWolfIsShaking - 0.4f) * 3.1415927f) * 7.0f);
			for (int_t i = 0; i < count; ++i)
			{
				float dx = (rand.nextFloat() * 2.0f - 1.0f) * width * 0.5f;
				float dz = (rand.nextFloat() * 2.0f - 1.0f) * width * 0.5f;
				worldObj->spawnParticle("splash", posX + dx, y + 0.8f, posZ + dz, motionX, motionY, motionZ);
			}
		}
	}
}

bool EntityWolf::getWolfShaking()
{
	return isWolfShaking;
}

float EntityWolf::getShadingWhileShaking(float f)
{
	return 0.75f + (prevTimeWolfIsShaking + (timeWolfIsShaking - prevTimeWolfIsShaking) * f) / 2.0f * 0.25f;
}

float EntityWolf::getShakeAngle(float f, float f1)
{
	float f2 = (prevTimeWolfIsShaking + (timeWolfIsShaking - prevTimeWolfIsShaking) * f + f1) / 1.8f;
	if (f2 < 0.0f)
	{
		f2 = 0.0f;
	}
	else if (f2 > 1.0f)
	{
		f2 = 1.0f;
	}
	return MathHelper::sin(f2 * 3.1415927f) * MathHelper::sin(f2 * 3.1415927f * 11.0f) * 0.15f * 3.1415927f;
}

float EntityWolf::getInterestedAngle(float f)
{
	return (field_25054_c + (field_25048_b - field_25054_c) * f) * 0.15f * 3.1415927f;
}

float EntityWolf::getEyeHeight()
{
	return height * 0.8f;
}

int_t EntityWolf::getVerticalFaceSpeed()
{
	if (isWolfSitting())
	{
		return 20;
	}
	return EntityAnimal::getVerticalFaceSpeed();
}

void EntityWolf::getPathOrWalkableBlock(Entity *entity, float f)
{
	PathEntity *pathentity = worldObj->getPathToEntity(this, entity, 16.0f);
#if PLATFORM_FLOAT_ENTITY_AI_MATH
	if (pathentity == nullptr && f > 144.0f)
#else
	if (pathentity == nullptr && f > 12.0f)
#endif
	{
		int i = MathHelper::floor_double(entity->posX) - 2;
		int j = MathHelper::floor_double(entity->posZ) - 2;
		int k = MathHelper::floor_double(entity->boundingBox->minY);
		for (int l = 0; l <= 4; l++)
		{
			for (int i1 = 0; i1 <= 4; i1++)
			{
				if ((l < 1 || i1 < 1 || l > 3 || i1 > 3) && worldObj->isBlockNormalCube(i + l, k - 1, j + i1) && !worldObj->isBlockNormalCube(i + l, k, j + i1) && !worldObj->isBlockNormalCube(i + l, k + 1, j + i1))
				{
					setLocationAndAngles((float)(i + l) + 0.5f, (double)k, (float)(j + i1) + 0.5f, rotationYaw, rotationPitch);
					return;
				}
			}
		}
	}
	else
	{
		setPathToEntity(pathentity);
	}
}

bool EntityWolf::isMovementCeased()
{
	return isWolfSitting() || field_25052_g;
}

bool EntityWolf::attackEntityFrom(Entity *entity, int_t i)
{
	if (aiSit != nullptr)
		aiSit->setSitting(false);
	if (entity != nullptr && !entity->isPlayer() && dynamic_cast<EntityArrow *>(entity) == nullptr)
		i = (i + 1) / 2;
	return EntityTameable::attackEntityFrom(entity, i);
}

bool EntityWolf::attackEntityFrom(const DamageSource &source, int_t damage)
{
	if (aiSit != nullptr)
		aiSit->setSitting(false);
	Entity *attacker = source.getEntity();
	if (attacker != nullptr && dynamic_cast<EntityPlayer *>(attacker) == nullptr && dynamic_cast<EntityArrow *>(attacker) == nullptr)
		damage = (damage + 1) / 2;
	return EntityTameable::attackEntityFrom(source, damage);
}

Entity *EntityWolf::findPlayerToAttack()
{
	if (isWolfAngry())
	{
		return worldObj->getClosestPlayerToEntity(this, 16.0);
	}
	return nullptr;
}

void EntityWolf::attackEntity(Entity *entity, float f)
{
	if (f > 2.0f && f < 6.0f && rand.nextInt(10) == 0)
	{
		if (onGround)
		{
#if PLATFORM_FLOAT_ENTITY_AI_MATH
			const float dx = (float)(entity->posX - posX);
			const float dz = (float)(entity->posZ - posZ);
			const float distance = MathHelper::sqrt_float(dx * dx + dz * dz);
			if (distance > 0.0f)
			{
				motionX = (double)((dx / distance) * 0.4f) + motionX * 0.20000000298023224;
				motionZ = (double)((dz / distance) * 0.4f) + motionZ * 0.20000000298023224;
			}
#else
			double dx = entity->posX - posX;
			double dz = entity->posZ - posZ;
			float distance = MathHelper::sqrt_double(dx * dx + dz * dz);
			if (distance > 0.0f)
			{
				motionX = (dx / distance) * 0.5 * 0.80000001192092896 + motionX * 0.20000000298023224;
				motionZ = (dz / distance) * 0.5 * 0.80000001192092896 + motionZ * 0.20000000298023224;
			}
#endif
			motionY = 0.40000000596046448;
		}
	}
	else if (f < 1.5f && entity->boundingBox->maxY > boundingBox->minY && entity->boundingBox->minY < boundingBox->maxY)
	{
		attackTime = 20;
		entity->attackEntityFrom(DamageSource::causeMobDamage(this), isWolfTamed() ? 4 : 2);
	}
}

bool EntityWolf::interact(EntityPlayer *entityplayer)
{
	if (entityplayer == nullptr || entityplayer->inventory == nullptr)
		return false;
	ItemStack *itemstack = entityplayer->inventory->getCurrentItem();
	if (!isTamed())
	{
		if (itemstack != nullptr && Item::bone != nullptr && itemstack->itemID == Item::bone->shiftedIndex && !isWolfAngry())
		{
			if (!entityplayer->capabilities.isCreativeMode)
			{
				--itemstack->stackSize;
				if (itemstack->stackSize <= 0)
				{
					int_t slot = entityplayer->inventory->currentItem;
					entityplayer->inventory->setInventorySlotContents(slot, nullptr);
					itemstack = nullptr;
				}
			}
			if (!worldObj->multiplayerWorld)
			{
				if (rand.nextInt(3) == 0)
				{
					setTamed(true);
					setPathToEntity(nullptr);
					setAttackTarget(nullptr);
					if (aiSit != nullptr)
						aiSit->setSitting(true);
					setHealth(20);
					setOwner(entityplayer->username);
					showHeartsOrSmokeFX(true);
					worldObj->setEntityState(this, (byte_t)7);
				}
				else
				{
					showHeartsOrSmokeFX(false);
					worldObj->setEntityState(this, (byte_t)6);
				}
			}
			return true;
		}
	}
	else
	{
		ItemFood *food = itemstack != nullptr ? dynamic_cast<ItemFood *>(itemstack->getItem()) : nullptr;
		if (food != nullptr && food->getIsWolfsFavoriteMeat() && dataWatcher->getWatchableObjectInt(18) < 20)
		{
			if (!entityplayer->capabilities.isCreativeMode)
			{
				--itemstack->stackSize;
				if (itemstack->stackSize <= 0)
				{
					int_t slot = entityplayer->inventory->currentItem;
					entityplayer->inventory->setInventorySlotContents(slot, nullptr);
					itemstack = nullptr;
				}
			}
			heal(food->getHealAmount());
			return true;
		}
		if (String::equalsIgnoreCaseJava(entityplayer->username, getOwnerName()) && !worldObj->multiplayerWorld && !isWheat(itemstack))
		{
			if (aiSit != nullptr)
				aiSit->setSitting(!isSitting());
			isJumping = false;
			setPathToEntity(nullptr);
			return true;
		}
	}
	return EntityTameable::interact(entityplayer);
}

void EntityWolf::showHeartsOrSmokeFX(bool flag)
{
	jstring s = flag ? "heart" : "smoke";
	for (int i = 0; i < 7; i++)
	{
		double d = rand.nextGaussian() * 0.02;
		double d1 = rand.nextGaussian() * 0.02;
		double d2 = rand.nextGaussian() * 0.02;
		const double particleX = posX + (double)(rand.nextFloat() * width * 2.0f) - (double)width;
		const double particleY = posY + 0.5 + (double)(rand.nextFloat() * height);
		const double particleZ = posZ + (double)(rand.nextFloat() * width * 2.0f) - (double)width;
		worldObj->spawnParticle(s, particleX, particleY, particleZ, d, d1, d2);
	}
}

void EntityWolf::handleHealthUpdate(byte_t byte0)
{
	if (byte0 == 7)
		showHeartsOrSmokeFX(true);
	else if (byte0 == 6)
		showHeartsOrSmokeFX(false);
	else if (byte0 == 8)
	{
		field_25052_g = true;
		timeWolfIsShaking = 0.0f;
		prevTimeWolfIsShaking = 0.0f;
	}
	else
		EntityTameable::handleHealthUpdate(byte0);
}

float EntityWolf::setTailRotation()
{
	if (isWolfAngry())
	{
		return 1.53938f;
	}
	if (isWolfTamed())
	{
		return (0.55f - (float)(20 - dataWatcher->getWatchableObjectInt(18)) * 0.02f) * 3.1415927f;
	}
	return 0.6283185f;
}

int_t EntityWolf::getMaxSpawnedInChunk()
{
	return 8;
}

jstring EntityWolf::getWolfOwner()
{
	return getOwnerName();
}

void EntityWolf::setWolfOwner(const jstring &s)
{
	setOwner(s);
}

bool EntityWolf::isWolfSitting()
{
	return isSitting();
}

void EntityWolf::setWolfSitting(bool flag)
{
	setSitting(flag);
	if (aiSit != nullptr)
		aiSit->setSitting(flag);
}

bool EntityWolf::isWolfAngry()
{
	return (dataWatcher->getWatchableObjectByte(16) & 2) != 0;
}

void EntityWolf::setWolfAngry(bool flag)
{
	byte_t byte0 = dataWatcher->getWatchableObjectByte(16);
	if (flag)
	{
		dataWatcher->updateObject(16, (byte_t)(byte0 | 2));
	}
	else
	{
		dataWatcher->updateObject(16, (byte_t)(byte0 & ~2));
	}
}

bool EntityWolf::isWolfTamed()
{
	return isTamed();
}

void EntityWolf::setWolfTamed(bool flag)
{
	setTamed(flag);
}
