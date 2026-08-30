#include "EntityItem.h"
#include "DamageSource.h"
#include "java/Math.h"
#include "java/Arithmetic.h"

#include <cmath>

#include "AchievementList.h"
#include "Block.h"
#include "EntityPlayer.h"
#include "InventoryPlayer.h"
#include "Item.h"
#include "ItemStack.h"
#include "MathHelper.h"
#include "Material.h"
#include "NBTTagCompound.h"
#include "World.h"
#include "AxisAlignedBB.h"

EntityItem::EntityItem(World *world, double d, double d1, double d2, ItemStack *itemstack)
	: Entity(world)
{
	ensureEntityInit();
	age = 0;
	health = 5;
	hoverStart = (float)(Math::random() * 3.1415926535897931 * 2.0);
	setSize(0.25f, 0.25f);
	yOffset = height / 2.0f;
	setPosition(d, d1, d2);
	item = itemstack;
	rotationYaw = (float)(Math::random() * 360.0);
	motionX = (float)(Math::random() * 0.20000000298023224 - 0.10000000149011612);
	motionY = 0.20000000298023224;
	motionZ = (float)(Math::random() * 0.20000000298023224 - 0.10000000149011612);
}

EntityItem::EntityItem(World *world)
	: Entity(world)
{
	ensureEntityInit();
	item = nullptr;
	age = 0;
	health = 5;
	hoverStart = (float)(Math::random() * 3.1415926535897931 * 2.0);
	setSize(0.25f, 0.25f);
	yOffset = height / 2.0f;
}

EntityItem::~EntityItem()
{
	delete item;
}

bool EntityItem::canTriggerWalking()
{
	return false;
}

void EntityItem::entityInit()
{
}

void EntityItem::onUpdate()
{
	Entity::onUpdate();
	if (delayBeforeCanPickup > 0)
	{
		delayBeforeCanPickup--;
	}
	prevPosX = posX;
	prevPosY = posY;
	prevPosZ = posZ;
	motionY -= 0.039999999105930328;
	if (worldObj->getBlockMaterial(MathHelper::floor_double(posX), MathHelper::floor_double(posY), MathHelper::floor_double(posZ)) == Material::lava)
	{
		motionY = 0.20000000298023224;
		motionX = rand.nextFloatDifference() * 0.2f;
		motionZ = rand.nextFloatDifference() * 0.2f;
		worldObj->playSoundAtEntity(this, "random.fizz", 0.4f, 2.0f + rand.nextFloat() * 0.4f);
	}
	pushOutOfBlocks(posX, (boundingBox->minY + boundingBox->maxY) / 2.0, posZ);
	moveEntity(motionX, motionY, motionZ);
	float f = 0.98f;
	if (onGround)
	{
		f = 0.1f * 0.1f * 58.8f;
		int i = worldObj->getBlockId(MathHelper::floor_double(posX), MathHelper::floor_double(boundingBox->minY) - 1, MathHelper::floor_double(posZ));
		if (i > 0)
		{
			f = Block::blocksList[i]->slipperiness * 0.98f;
		}
	}
	motionX *= (double)f;
	motionY *= 0.98000001907348633;
	motionZ *= (double)f;
	if (onGround)
	{
		motionY *= -0.5;
	}
	age++;
	if (age >= 6000)
	{
		setEntityDead();
	}
}

bool EntityItem::handleWaterMovement()
{
	return worldObj->handleMaterialAcceleration(boundingBox, Material::water, this);
}

void EntityItem::dealFireDamage(int_t i)
{
	attackEntityFrom(DamageSource::inFire, i);
}

bool EntityItem::attackEntityFrom(Entity *entity, int_t i)
{
	setBeenAttacked();
	health -= i;
	if (health <= 0)
	{
		setEntityDead();
	}
	return false;
}

bool EntityItem::attackEntityFrom(const DamageSource &source, int_t damage)
{
	return attackEntityFrom(source.getEntity(), damage);
}

void EntityItem::writeEntityToNBT(NBTTagCompound *nbttagcompound)
{
	nbttagcompound->setShort("Health", static_cast<short_t>(JavaArithmetic::byteFromBits(static_cast<ubyte_t>(health))));
	nbttagcompound->setShort("Age", (short)age);
	nbttagcompound->setCompoundTag("Item", item->writeToNBT(new NBTTagCompound()));
}

void EntityItem::readEntityFromNBT(NBTTagCompound *nbttagcompound)
{
	health = nbttagcompound->getShort("Health") & 0xFF;
	age = nbttagcompound->getShort("Age");
	NBTTagCompound *nbttagcompound1 = nbttagcompound->getCompoundTag("Item");
	delete item;
	item = ItemStack::loadItemStackFromNBT(nbttagcompound1);
	if (item == nullptr)
		setDead();
}

void EntityItem::onCollideWithPlayer(EntityPlayer *entityplayer)
{
	if (worldObj->multiplayerWorld)
	{
		return;
	}
	int i = item->stackSize;
	if (delayBeforeCanPickup == 0 && entityplayer->inventory->addItemStackToInventory(item))
	{
		if (item->itemID == Block::wood->blockID)
		{
			entityplayer->triggerAchievement(AchievementList::mineWood);
		}
		if (item->itemID == Item::leather->shiftedIndex)
		{
			entityplayer->triggerAchievement(AchievementList::killCow);
		}
		if (item->itemID == Item::diamond->shiftedIndex)
		{
			entityplayer->triggerAchievement(AchievementList::diamonds);
		}
		if (item->itemID == Item::blazeRod->shiftedIndex)
		{
			entityplayer->triggerAchievement(AchievementList::blazeRod);
		}
		worldObj->playSoundAtEntity(this, "random.pop", 0.2f, (rand.nextFloatDifference() * 0.7f + 1.0f) * 2.0f);
		entityplayer->onItemPickup(this, i);
		if (item->stackSize <= 0)
		{
			setEntityDead();
		}
	}
}

bool EntityItem::canAttackWithItem()
{
	return false;
}
