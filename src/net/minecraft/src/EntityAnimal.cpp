#include "EntityAnimal.h"
#include "DamageSource.h"

#include <cmath>

#include "AxisAlignedBB.h"
#include "Block.h"
#include "EntityPlayer.h"
#include "InventoryPlayer.h"
#include "Item.h"
#include "ItemStack.h"
#include "MathHelper.h"
#include "java/Math.h"
#include "NBTTagCompound.h"
#include "PlayerCapabilities.h"
#include "World.h"

EntityAnimal::EntityAnimal(World *world)
	: EntityAgeable(world), inLove(0), breeding(0)
{
}

void EntityAnimal::updateAITick()
{
	if (getGrowingAge() != 0)
		inLove = 0;
	EntityAgeable::updateAITick();
}

void EntityAnimal::onLivingUpdate()
{
	EntityAgeable::onLivingUpdate();

	if (getGrowingAge() != 0)
		inLove = 0;

	if (inLove > 0)
	{
		--inLove;
		if (inLove % 10 == 0)
		{
			double motionX = rand.nextGaussian() * 0.02;
			double motionY = rand.nextGaussian() * 0.02;
			double motionZ = rand.nextGaussian() * 0.02;
			const double particleX = posX + (double)(rand.nextFloat() * width * 2.0f) - (double)width;
			const double particleY = posY + 0.5 + (double)(rand.nextFloat() * height);
			const double particleZ = posZ + (double)(rand.nextFloat() * width * 2.0f) - (double)width;
			worldObj->spawnParticle("heart", particleX, particleY, particleZ, motionX, motionY, motionZ);
		}
	}
	else
	{
		breeding = 0;
	}
}

void EntityAnimal::attackEntity(Entity *entity, float distance)
{
	if (entity == nullptr)
		return;

	EntityPlayer *player = dynamic_cast<EntityPlayer *>(entity);
	if (player != nullptr)
	{
		if (distance < 3.0f)
		{
			const double dx = entity->posX - posX;
			const double dz = entity->posZ - posZ;
			rotationYaw = (float)(JavaMath::atan2(dz, dx) * 180.0 / (double)3.14159274f) - 90.0f;
			hasAttacked = true;
		}

		ItemStack *held = player->getCurrentEquippedItem();
		if (held == nullptr || !isWheat(held))
			playerToAttack = nullptr;
		return;
	}

	EntityAnimal *animal = dynamic_cast<EntityAnimal *>(entity);
	if (animal == nullptr)
		return;

	if (getGrowingAge() > 0 && animal->getGrowingAge() < 0)
	{
		if ((double)distance < 2.5)
			hasAttacked = true;
	}
	else if (inLove > 0 && animal->inLove > 0)
	{
		if (animal->playerToAttack == nullptr)
			animal->playerToAttack = this;

		if (animal->playerToAttack == this && (double)distance < 3.5)
		{
			++animal->inLove;
			++inLove;
			++breeding;
			if (breeding % 4 == 0)
			{
				const double particleX = posX + (double)(rand.nextFloat() * width * 2.0f) - (double)width;
				const double particleY = posY + 0.5 + (double)(rand.nextFloat() * height);
				const double particleZ = posZ + (double)(rand.nextFloat() * width * 2.0f) - (double)width;
				worldObj->spawnParticle("heart", particleX, particleY, particleZ, 0.0, 0.0, 0.0);
			}
			if (breeding == 60)
				procreate(animal);
		}
		else
		{
			breeding = 0;
		}
	}
	else
	{
		breeding = 0;
		playerToAttack = nullptr;
	}
}

void EntityAnimal::procreate(EntityAnimal *mate)
{
	if (mate == nullptr)
		return;
	EntityAnimal *child = spawnBabyAnimal(mate);
	if (child == nullptr)
		return;

	setGrowingAge(6000);
	mate->setGrowingAge(6000);
	inLove = 0;
	breeding = 0;
	playerToAttack = nullptr;
	mate->playerToAttack = nullptr;
	mate->breeding = 0;
	mate->inLove = 0;
	child->setGrowingAge(-24000);
	child->setLocationAndAngles(posX, posY, posZ, rotationYaw, rotationPitch);

	for (int_t i = 0; i < 7; ++i)
	{
		const double motionX = rand.nextGaussian() * 0.02;
		const double motionY = rand.nextGaussian() * 0.02;
		const double motionZ = rand.nextGaussian() * 0.02;
		const double particleX = posX + (double)(rand.nextFloat() * width * 2.0f) - (double)width;
		const double particleY = posY + 0.5 + (double)(rand.nextFloat() * height);
		const double particleZ = posZ + (double)(rand.nextFloat() * width * 2.0f) - (double)width;
		worldObj->spawnParticle("heart", particleX, particleY, particleZ, motionX, motionY, motionZ);
	}

	if (!worldObj->spawnEntityInWorld(child))
		delete child;
}

void EntityAnimal::attackBlockedEntity(Entity *, float)
{
}

Entity *EntityAnimal::findPlayerToAttack()
{
	if (fleeingTick > 0)
		return nullptr;

	const double range = 8.0;
	AxisAlignedBB *searchBox = boundingBox->expand(range, range, range);
	if (inLove > 0)
	{
		const std::vector<Entity *> &animals = worldObj->getEntitiesWithinAABB(typeid(*this), searchBox);
		for (Entity *entity : animals)
		{
			EntityAnimal *animal = dynamic_cast<EntityAnimal *>(entity);
			if (animal != nullptr && animal != this && animal->inLove > 0)
				return animal;
		}
	}
	else if (getGrowingAge() == 0)
	{
		const std::vector<Entity *> &players = worldObj->getEntitiesWithinAABB(typeid(EntityPlayer), searchBox);
		for (Entity *entity : players)
		{
			EntityPlayer *player = dynamic_cast<EntityPlayer *>(entity);
			if (player != nullptr)
			{
				ItemStack *held = player->getCurrentEquippedItem();
				if (held != nullptr && isWheat(held))
					return player;
			}
		}
	}
	else if (getGrowingAge() > 0)
	{
		const std::vector<Entity *> &animals = worldObj->getEntitiesWithinAABB(typeid(*this), searchBox);
		for (Entity *entity : animals)
		{
			EntityAnimal *animal = dynamic_cast<EntityAnimal *>(entity);
			if (animal != nullptr && animal != this && animal->getGrowingAge() < 0)
				return animal;
		}
	}

	return nullptr;
}

bool EntityAnimal::attackEntityFrom(Entity *entity, int_t damage)
{
	fleeingTick = 60;
	playerToAttack = nullptr;
	inLove = 0;
	return EntityAgeable::attackEntityFrom(entity, damage);
}

bool EntityAnimal::attackEntityFrom(const DamageSource &source, int_t damage)
{
	fleeingTick = 60;
	playerToAttack = nullptr;
	inLove = 0;
	return EntityAgeable::attackEntityFrom(source, damage);
}

float EntityAnimal::getBlockPathWeight(int_t i, int_t j, int_t k)
{
	if (worldObj->getBlockId(i, j - 1, k) == ((Block *)Block::grass)->blockID)
		return 10.0f;
	return worldObj->getLightBrightness(i, j, k) - 0.5f;
}

void EntityAnimal::writeEntityToNBT(NBTTagCompound *nbttagcompound)
{
	EntityAgeable::writeEntityToNBT(nbttagcompound);
	nbttagcompound->setInteger("InLove", inLove);
}

void EntityAnimal::readEntityFromNBT(NBTTagCompound *nbttagcompound)
{
	EntityAgeable::readEntityFromNBT(nbttagcompound);
	inLove = nbttagcompound->getInteger("InLove");
}

bool EntityAnimal::getCanSpawnHere()
{
	int_t i = MathHelper::floor_double(posX);
	int_t j = MathHelper::floor_double(boundingBox->minY);
	int_t k = MathHelper::floor_double(posZ);
	return worldObj->getBlockId(i, j - 1, k) == ((Block *)Block::grass)->blockID &&
		worldObj->getFullBlockLightValue(i, j, k) > 8 && EntityAgeable::getCanSpawnHere();
}

bool EntityAnimal::canDespawn()
{
	return false;
}

int_t EntityAnimal::getExperiencePoints(EntityPlayer *player)
{
	(void)player;
	return 1 + worldObj->rand.nextInt(3);
}

bool EntityAnimal::isWheat(ItemStack *itemstack) const
{
	return itemstack != nullptr && itemstack->itemID == Item::wheat->shiftedIndex;
}

bool EntityAnimal::interact(EntityPlayer *entityplayer)
{
	ItemStack *itemstack = entityplayer != nullptr ? entityplayer->inventory->getCurrentItem() : nullptr;
	if (itemstack == nullptr || !isWheat(itemstack) || getGrowingAge() != 0)
		return EntityAgeable::interact(entityplayer);

	if (!entityplayer->capabilities.isCreativeMode)
	{
		--itemstack->stackSize;
		if (itemstack->stackSize <= 0)
		{
			const int_t slot = entityplayer->inventory->currentItem;
			entityplayer->inventory->setInventorySlotContents(slot, nullptr);
		}
	}

	inLove = 600;
	playerToAttack = nullptr;
	for (int_t i = 0; i < 7; ++i)
	{
		double motionX = rand.nextGaussian() * 0.02;
		double motionY = rand.nextGaussian() * 0.02;
		double motionZ = rand.nextGaussian() * 0.02;
		const double particleX = posX + (double)(rand.nextFloat() * width * 2.0f) - (double)width;
		const double particleY = posY + 0.5 + (double)(rand.nextFloat() * height);
		const double particleZ = posZ + (double)(rand.nextFloat() * width * 2.0f) - (double)width;
		worldObj->spawnParticle("heart", particleX, particleY, particleZ, motionX, motionY, motionZ);
	}
	return true;
}

EntityAnimal *EntityAnimal::spawnBabyAnimal(EntityAnimal *mate)
{
	(void)mate;
	return nullptr;
}

bool EntityAnimal::isInLove() const
{
	return inLove > 0;
}

void EntityAnimal::resetInLove()
{
	inLove = 0;
}

bool EntityAnimal::canMateWith(EntityAnimal *mate) const
{
	return mate != nullptr && mate != this && typeid(*mate) == typeid(*this) && isInLove() && mate->isInLove();
}
