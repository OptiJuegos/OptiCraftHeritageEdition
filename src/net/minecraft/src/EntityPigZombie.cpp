#include "EntityPigZombie.h"
#include "java/Arithmetic.h"

#include "EntityPlayer.h"
#include "Item.h"
#include "ItemStack.h"
#include "NBTTagCompound.h"
#include "World.h"
#include "AxisAlignedBB.h"
#include "DamageSource.h"
#include "EnchantmentHelper.h"

ItemStack *EntityPigZombie::defaultHeldItem = nullptr;

EntityPigZombie::EntityPigZombie(World *world)
	: EntityZombie(world)
{
	angerLevel = 0;
	randomSoundDelay = 0;
	texture = "/mob/pigzombie.png";
	moveSpeed = 0.5f;
	attackStrength = 5;
	immuneToFire = true;
	if (defaultHeldItem == nullptr)
	{
		defaultHeldItem = new ItemStack(Item::swordGold, 1);
	}
}

bool EntityPigZombie::isAIEnabled()
{
	return false;
}

void EntityPigZombie::onUpdate()
{
	moveSpeed = playerToAttack == nullptr ? 0.5f : 0.95f;
	if (randomSoundDelay > 0 && --randomSoundDelay == 0)
	{
		worldObj->playSoundAtEntity(this, "mob.zombiepig.zpigangry", getSoundVolume() * 2.0f, (rand.nextFloatDifference() * 0.2f + 1.0f) * 1.8f);
	}
	EntityZombie::onUpdate();
}

bool EntityPigZombie::getCanSpawnHere()
{
	return worldObj->difficultySetting > 0 && worldObj->checkIfAABBIsClear(boundingBox) && worldObj->getCollidingBoundingBoxes(this, boundingBox).size() == 0 && !worldObj->getIsAnyLiquid(boundingBox);
}

void EntityPigZombie::writeEntityToNBT(NBTTagCompound *nbttagcompound)
{
	EntityZombie::writeEntityToNBT(nbttagcompound);
	nbttagcompound->setShort("Anger", JavaArithmetic::shortFromBits(static_cast<ushort_t>(angerLevel)));
}

void EntityPigZombie::readEntityFromNBT(NBTTagCompound *nbttagcompound)
{
	EntityZombie::readEntityFromNBT(nbttagcompound);
	angerLevel = nbttagcompound->getShort("Anger");
}

Entity *EntityPigZombie::findPlayerToAttack()
{
	if (angerLevel == 0)
	{
		return nullptr;
	}
	return EntityZombie::findPlayerToAttack();
}

void EntityPigZombie::onLivingUpdate()
{
	EntityZombie::onLivingUpdate();
}

bool EntityPigZombie::attackEntityFrom(Entity *entity, int_t damage)
{
	if (entity == nullptr)
		return EntityZombie::attackEntityFrom(entity, damage);
	DamageSource source = entity->isPlayer()
		? DamageSource::causePlayerDamage(static_cast<EntityPlayer *>(entity))
		: DamageSource::causeMobDamage(dynamic_cast<EntityLiving *>(entity));
	return attackEntityFrom(source, damage);
}

bool EntityPigZombie::attackEntityFrom(const DamageSource &source, int_t damage)
{
	Entity *entity = source.getEntity();
	if (dynamic_cast<EntityPlayer *>(entity) != nullptr)
	{
		const auto &list = worldObj->getEntitiesWithinAABBExcludingEntity(this, boundingBox->expand(32.0, 32.0, 32.0));
		for (Entity *other : list)
		{
			EntityPigZombie *pigZombie = dynamic_cast<EntityPigZombie *>(other);
			if (pigZombie != nullptr)
				pigZombie->becomeAngryAt(entity);
		}
		becomeAngryAt(entity);
	}
	return EntityZombie::attackEntityFrom(source, damage);
}

void EntityPigZombie::becomeAngryAt(Entity *entity)
{
	playerToAttack = entity;
	angerLevel = 400 + rand.nextInt(400);
	randomSoundDelay = rand.nextInt(40);
}

jstring EntityPigZombie::getLivingSound() { return "mob.zombiepig.zpig"; }
jstring EntityPigZombie::getHurtSound()   { return "mob.zombiepig.zpighurt"; }
jstring EntityPigZombie::getDeathSound()  { return "mob.zombiepig.zpigdeath"; }

int_t EntityPigZombie::getDropItemId()
{
	return Item::rottenFlesh != nullptr ? Item::rottenFlesh->shiftedIndex : 0;
}

void EntityPigZombie::dropFewItems(bool, int_t lootingLevel)
{
	int_t count = rand.nextInt(2 + lootingLevel);
	for (int_t i = 0; i < count; ++i)
		if (Item::rottenFlesh != nullptr)
			dropItem(Item::rottenFlesh->shiftedIndex, 1);
	count = rand.nextInt(2 + lootingLevel);
	for (int_t i = 0; i < count; ++i)
		if (Item::goldNugget != nullptr)
			dropItem(Item::goldNugget->shiftedIndex, 1);
}

void EntityPigZombie::dropRareDrop(int_t lootingRoll)
{
	if (lootingRoll > 0)
	{
		if (Item::swordGold == nullptr)
			return;
		ItemStack *stack = new ItemStack(Item::swordGold);
		EnchantmentHelper::addRandomEnchantment(rand, stack, 5);
		entityDropItem(stack, 0.0f);
		return;
	}
	switch (rand.nextInt(3))
	{
	case 0: if (Item::ingotGold != nullptr) dropItem(Item::ingotGold->shiftedIndex, 1); break;
	case 1: if (Item::swordGold != nullptr) dropItem(Item::swordGold->shiftedIndex, 1); break;
	case 2: if (Item::helmetGold != nullptr) dropItem(Item::helmetGold->shiftedIndex, 1); break;
	}
}

ItemStack *EntityPigZombie::getHeldItem()
{
	return defaultHeldItem;
}
