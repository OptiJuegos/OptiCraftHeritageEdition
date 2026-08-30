#include "EntitySkeleton.h"

#include <cmath>
#include "java/Math.h"

#include "EntityArrow.h"
#include "EntityAIArrowAttack.h"
#include "EntityAIFleeSun.h"
#include "EntityAIHurtByTarget.h"
#include "EntityAILookIdle.h"
#include "EntityAINearestAttackableTarget.h"
#include "EntityAIRestrictSun.h"
#include "EntityAISwimming.h"
#include "EntityAIWander.h"
#include "EntityAIWatchClosest.h"
#include "EntityPlayer.h"
#include "AchievementList.h"
#include "DamageSource.h"
#include "EnchantmentHelper.h"
#include "Item.h"
#include "ItemStack.h"
#include "MathHelper.h"
#include "NBTTagCompound.h"
#include "World.h"
#include "platform/PlatformTuning.h"

ItemStack *EntitySkeleton::defaultHeldItem = nullptr;

EntitySkeleton::EntitySkeleton(World *world)
	: EntityMob(world)
{
	texture = "/mob/skeleton.png";
	moveSpeed = 0.25f;
	tasks.addTask(1, new EntityAISwimming(this));
	tasks.addTask(2, new EntityAIRestrictSun(this));
	tasks.addTask(3, new EntityAIFleeSun(this, moveSpeed));
	tasks.addTask(4, new EntityAIArrowAttack(this, moveSpeed, 1, 60));
	tasks.addTask(5, new EntityAIWander(this, moveSpeed));
	tasks.addTask(6, new EntityAIWatchClosest(this, typeid(EntityPlayer), 8.0f));
	tasks.addTask(6, new EntityAILookIdle(this));
	targetTasks.addTask(1, new EntityAIHurtByTarget(this, false));
	targetTasks.addTask(2, new EntityAINearestAttackableTarget(this, typeid(EntityPlayer), 16.0f, 0, true));
	if (defaultHeldItem == nullptr)
	{
		defaultHeldItem = new ItemStack(Item::bow, 1);
	}
}

void EntitySkeleton::onLivingUpdate()
{
	if (worldObj->isDaytime() && !worldObj->multiplayerWorld)
	{
		float f = getEntityBrightness(1.0f);
		if (f > 0.5f && worldObj->canBlockSeeTheSky(MathHelper::floor_double(posX), MathHelper::floor_double(posY), MathHelper::floor_double(posZ)) && rand.nextFloat() * 30.0f < (f - 0.4f) * 2.0f)
		{
			setFire(8);
		}
	}
	EntityMob::onLivingUpdate();
}

void EntitySkeleton::onDeath(const DamageSource &source)
{
	EntityMob::onDeath(source);
	EntityArrow *arrow = dynamic_cast<EntityArrow *>(source.getSourceOfDamage());
	EntityPlayer *player = dynamic_cast<EntityPlayer *>(source.getEntity());
	if (arrow != nullptr && player != nullptr)
	{
		const double dx = player->posX - posX;
		const double dz = player->posZ - posZ;
		if (dx * dx + dz * dz >= 2500.0 && AchievementList::snipeSkeleton != nullptr)
			player->triggerAchievement(AchievementList::snipeSkeleton);
	}
}

bool EntitySkeleton::isAIEnabled()
{
	return true;
}

jstring EntitySkeleton::getLivingSound() { return "mob.skeleton"; }
jstring EntitySkeleton::getHurtSound()   { return "mob.skeletonhurt"; }
jstring EntitySkeleton::getDeathSound()  { return "mob.skeletonhurt"; }

void EntitySkeleton::writeEntityToNBT(NBTTagCompound *nbttagcompound)
{
	EntityMob::writeEntityToNBT(nbttagcompound);
}

void EntitySkeleton::readEntityFromNBT(NBTTagCompound *nbttagcompound)
{
	EntityMob::readEntityFromNBT(nbttagcompound);
}

int_t EntitySkeleton::getDropItemId()
{
	return Item::arrow->shiftedIndex;
}

void EntitySkeleton::dropFewItems(bool, int_t lootingLevel)
{
	int_t count = rand.nextInt(3 + lootingLevel);
	for (int_t i = 0; i < count; ++i)
		dropItem(Item::arrow->shiftedIndex, 1);
	count = rand.nextInt(3 + lootingLevel);
	for (int_t i = 0; i < count; ++i)
		dropItem(Item::bone->shiftedIndex, 1);
}

void EntitySkeleton::dropRareDrop(int_t lootingRoll)
{
	if (Item::bow == nullptr)
		return;
	if (lootingRoll > 0)
	{
		ItemStack *stack = new ItemStack(Item::bow);
		EnchantmentHelper::addRandomEnchantment(rand, stack, 5);
		entityDropItem(stack, 0.0f);
	}
	else
	{
		dropItem(Item::bow->shiftedIndex, 1);
	}
}

ItemStack *EntitySkeleton::getHeldItem()
{
	return defaultHeldItem;
}

EnumCreatureAttribute EntitySkeleton::getCreatureAttribute() const
{
	return EnumCreatureAttribute::UNDEAD;
}
