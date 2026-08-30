#include "EntitySheep.h"
#include "java/Arithmetic.h"

#include "Block.h"
#include "DataWatcher.h"
#include "EntityAIEatGrass.h"
#include "EntityAIFollowParent.h"
#include "EntityAILookIdle.h"
#include "EntityAIMate.h"
#include "EntityAIPanic.h"
#include "EntityAISwimming.h"
#include "EntityAITempt.h"
#include "EntityAIWander.h"
#include "EntityAIWatchClosest.h"
#include "EntityItem.h"
#include "EntityPlayer.h"
#include "InventoryPlayer.h"
#include "Item.h"
#include "ItemStack.h"
#include "MathHelper.h"
#include "NBTTagCompound.h"
#include "PathNavigate.h"
#include "World.h"

const float EntitySheep::fleeceColorTable[16][3] = {
	{1.00f, 1.00f, 1.00f},
	{0.95f, 0.70f, 0.20f},
	{0.90f, 0.50f, 0.85f},
	{0.60f, 0.70f, 0.95f},
	{0.90f, 0.90f, 0.20f},
	{0.50f, 0.80f, 0.10f},
	{0.95f, 0.70f, 0.80f},
	{0.30f, 0.30f, 0.30f},
	{0.60f, 0.60f, 0.60f},
	{0.30f, 0.60f, 0.70f},
	{0.70f, 0.40f, 0.90f},
	{0.20f, 0.40f, 0.80f},
	{0.50f, 0.40f, 0.30f},
	{0.40f, 0.50f, 0.20f},
	{0.80f, 0.30f, 0.30f},
	{0.10f, 0.10f, 0.10f},
};

EntitySheep::EntitySheep(World *world)
	: EntityAnimal(world), sheepTimer(0), aiEatGrass(nullptr)
{
	ensureEntityInit();
	texture = "/mob/sheep.png";
	setSize(0.9f, 1.3f);
	health = 8;
	const float speed = 0.23f;
	if (getNavigator() != nullptr)
		getNavigator()->setAvoidsWater(true);
	tasks.addTask(0, new EntityAISwimming(this));
	tasks.addTask(1, new EntityAIPanic(this, 0.38f));
	tasks.addTask(2, new EntityAIMate(this, speed));
	tasks.addTask(3, new EntityAITempt(this, 0.25f, Item::wheat->shiftedIndex, false));
	tasks.addTask(4, new EntityAIFollowParent(this, 0.25f));
	aiEatGrass = new EntityAIEatGrass(this);
	tasks.addTask(5, aiEatGrass);
	tasks.addTask(6, new EntityAIWander(this, speed));
	tasks.addTask(7, new EntityAIWatchClosest(this, typeid(EntityPlayer), 6.0f));
	tasks.addTask(8, new EntityAILookIdle(this));
}

bool EntitySheep::isAIEnabled()
{
	return true;
}

int_t EntitySheep::getMaxHealth() const
{
	return 8;
}

void EntitySheep::entityInit()
{
	EntityAnimal::entityInit();
	dataWatcher->addObject(16, (byte_t)0);
}

void EntitySheep::updateAITasks()
{
	sheepTimer = aiEatGrass != nullptr ? aiEatGrass->getEatGrassTick() : 0;
	EntityAnimal::updateAITasks();
}

void EntitySheep::onLivingUpdate()
{
	if (worldObj->multiplayerWorld)
		sheepTimer = sheepTimer > 0 ? sheepTimer - 1 : 0;
	EntityAnimal::onLivingUpdate();
}

void EntitySheep::dropFewItems(bool, int_t lootingLevel)
{
	if (!getSheared())
		entityDropItem(new ItemStack(((Block *)Block::cloth)->blockID, 1, getFleeceColor()), 0.0f);
}

int_t EntitySheep::getDropItemId()
{
	return ((Block *)Block::cloth)->blockID;
}

void EntitySheep::handleHealthUpdate(byte_t byte0)
{
	if (byte0 == 10)
		sheepTimer = 40;
	else
		EntityAnimal::handleHealthUpdate(byte0);
}

float EntitySheep::getHeadRotationPointY(float partialTick) const
{
	if (sheepTimer <= 0)
		return 0.0f;
	if (sheepTimer >= 4 && sheepTimer <= 36)
		return 1.0f;
	if (sheepTimer < 4)
		return ((float)sheepTimer - partialTick) / 4.0f;
	return -((float)(sheepTimer - 40) - partialTick) / 4.0f;
}

float EntitySheep::getHeadRotationAngleX(float partialTick) const
{
	if (sheepTimer > 4 && sheepTimer <= 36)
	{
		float progress = ((float)(sheepTimer - 4) - partialTick) / 32.0f;
		return 3.14159274f * 0.2f + 3.14159274f * 0.07f * MathHelper::sin(progress * 28.7f);
	}
	return sheepTimer > 0 ? 3.14159274f * 0.2f : rotationPitch / (180.0f / 3.14159274f);
}

bool EntitySheep::interact(EntityPlayer *entityplayer)
{
	ItemStack *itemstack = entityplayer != nullptr ? entityplayer->inventory->getCurrentItem() : nullptr;
	if (itemstack != nullptr && itemstack->itemID == Item::shears->shiftedIndex && !getSheared() && !isChild())
	{
		if (!worldObj->multiplayerWorld)
		{
			setSheared(true);
			int_t count = 1 + rand.nextInt(3);
			for (int_t i = 0; i < count; ++i)
			{
				EntityItem *entityitem = entityDropItem(new ItemStack(((Block *)Block::cloth)->blockID, 1, getFleeceColor()), 1.0f);
				if (entityitem == nullptr)
					continue;
				entityitem->motionY += (double)(rand.nextFloat() * 0.05f);
				entityitem->motionX += (double)(rand.nextFloatDifference() * 0.1f);
				entityitem->motionZ += (double)(rand.nextFloatDifference() * 0.1f);
			}
		}
		itemstack->damageItem(1, entityplayer);
	}
	return EntityAnimal::interact(entityplayer);
}

void EntitySheep::writeEntityToNBT(NBTTagCompound *nbttagcompound)
{
	EntityAnimal::writeEntityToNBT(nbttagcompound);
	nbttagcompound->setBoolean("Sheared", getSheared());
	nbttagcompound->setByte("Color", JavaArithmetic::byteFromBits(static_cast<ubyte_t>(getFleeceColor())));
}

void EntitySheep::readEntityFromNBT(NBTTagCompound *nbttagcompound)
{
	EntityAnimal::readEntityFromNBT(nbttagcompound);
	setSheared(nbttagcompound->getBoolean("Sheared"));
	setFleeceColor(nbttagcompound->getByte("Color"));
}

jstring EntitySheep::getLivingSound() { return "mob.sheep"; }
jstring EntitySheep::getHurtSound() { return "mob.sheep"; }
jstring EntitySheep::getDeathSound() { return "mob.sheep"; }

int_t EntitySheep::getFleeceColor()
{
	return dataWatcher->getWatchableObjectByte(16) & 0xf;
}

void EntitySheep::setFleeceColor(int_t i)
{
	byte_t value = dataWatcher->getWatchableObjectByte(16);
	dataWatcher->updateObject(16, (byte_t)((value & 0xf0) | (i & 0xf)));
}

bool EntitySheep::getSheared()
{
	return (dataWatcher->getWatchableObjectByte(16) & 0x10) != 0;
}

void EntitySheep::setSheared(bool flag)
{
	byte_t value = dataWatcher->getWatchableObjectByte(16);
	dataWatcher->updateObject(16, flag ? (byte_t)(value | 0x10) : (byte_t)(value & 0xef));
}

int_t EntitySheep::getRandomFleeceColor(Random &random)
{
	int_t i = random.nextInt(100);
	if (i < 5)
		return 15;
	if (i < 10)
		return 7;
	if (i < 15)
		return 8;
	if (i < 18)
		return 12;
	return random.nextInt(500) == 0 ? 6 : 0;
}

EntityAnimal *EntitySheep::spawnBabyAnimal(EntityAnimal *mate)
{
	EntitySheep *other = dynamic_cast<EntitySheep *>(mate);
	EntitySheep *child = new EntitySheep(worldObj);
	if (other == nullptr || rand.nextBoolean())
		child->setFleeceColor(getFleeceColor());
	else
		child->setFleeceColor(other->getFleeceColor());
	return child;
}

void EntitySheep::eatGrassBonus()
{
	setSheared(false);
	if (isChild())
	{
		int_t age = getGrowingAge() + 1200;
		if (age > 0)
			age = 0;
		setGrowingAge(age);
	}
}
