#include "EntityOcelot.h"

#include "Block.h"
#include "BlockGrass.h"
#include "BlockLeaves.h"
#include "DataWatcher.h"
#include "DamageSource.h"
#include "EntityAIAvoidEntity.h"
#include "EntityAIFollowOwner.h"
#include "EntityAILeapAtTarget.h"
#include "EntityAIMate.h"
#include "EntityAISit.h"
#include "EntityAIOcelotAttack.h"
#include "EntityAIOcelotSit.h"
#include "EntityAISwimming.h"
#include "EntityAITargetNonTamed.h"
#include "EntityAITempt.h"
#include "EntityAIWander.h"
#include "EntityAIWatchClosest.h"
#include "EntityChicken.h"
#include "EntityPlayer.h"
#include "EntityMoveHelper.h"
#include "InventoryPlayer.h"
#include "Item.h"
#include "ItemStack.h"
#include "MathHelper.h"
#include "NBTTagCompound.h"
#include "PathNavigate.h"
#include "PlayerCapabilities.h"
#include "World.h"
#include "java/String.h"

EntityOcelot::EntityOcelot(World *world)
	: EntityTameable(world), aiTempt(nullptr)
{
	ensureEntityInit();
	texture = "/mob/ozelot.png";
	setSize(0.6f, 0.8f);
	setHealth(getMaxHealth());
	if (getNavigator() != nullptr)
		getNavigator()->setAvoidsWater(true);
	tasks.addTask(1, new EntityAISwimming(this));
	tasks.addTask(2, aiSit);
	aiTempt = new EntityAITempt(this, 0.18f, Item::fishRaw != nullptr ? Item::fishRaw->shiftedIndex : -1, true);
	tasks.addTask(3, aiTempt);
	tasks.addTask(4, new EntityAIAvoidEntity(this, typeid(EntityPlayer), 16.0f, 0.23f, 0.4f));
	tasks.addTask(5, new EntityAIFollowOwner(this, 0.3f, 10.0f, 5.0f));
	tasks.addTask(6, new EntityAIOcelotSit(this, 0.4f));
	tasks.addTask(7, new EntityAILeapAtTarget(this, 0.3f));
	tasks.addTask(8, new EntityAIOcelotAttack(this));
	tasks.addTask(9, new EntityAIMate(this, 0.23f));
	tasks.addTask(10, new EntityAIWander(this, 0.23f));
	tasks.addTask(11, new EntityAIWatchClosest(this, typeid(EntityPlayer), 10.0f));
	targetTasks.addTask(1, new EntityAITargetNonTamed(this, typeid(EntityChicken), 14.0f, 750, false));
}

void EntityOcelot::entityInit()
{
	EntityTameable::entityInit();
	dataWatcher->addObject(18, (byte_t)0);
}

bool EntityOcelot::isAIEnabled()
{
	return true;
}

int_t EntityOcelot::getMaxHealth() const
{
	return 10;
}

void EntityOcelot::updateAITick()
{
	EntityMoveHelper *moveHelper = getMoveHelper();
	if (moveHelper == nullptr || !moveHelper->isUpdating())
	{
		setSneaking(false);
		setSprinting(false);
		return;
	}
	float speed = moveHelper->getSpeed();
	if (speed == 0.18f)
	{
		setSneaking(true);
		setSprinting(false);
	}
	else if (speed == 0.4f)
	{
		setSneaking(false);
		setSprinting(true);
	}
	else
	{
		setSneaking(false);
		setSprinting(false);
	}
}

bool EntityOcelot::canDespawn()
{
	return !isTamed();
}

const char *EntityOcelot::getEntityTexture()
{
	switch (getCatType())
	{
	case 0: return "/mob/ozelot.png";
	case 1: return "/mob/cat_black.png";
	case 2: return "/mob/cat_red.png";
	case 3: return "/mob/cat_siamese.png";
	default: return EntityTameable::getEntityTexture();
	}
}

void EntityOcelot::fall(float)
{
}

void EntityOcelot::writeEntityToNBT(NBTTagCompound *compound)
{
	EntityTameable::writeEntityToNBT(compound);
	compound->setInteger("CatType", getCatType());
}

void EntityOcelot::readEntityFromNBT(NBTTagCompound *compound)
{
	EntityTameable::readEntityFromNBT(compound);
	setCatType(compound->getInteger("CatType"));
}

jstring EntityOcelot::getLivingSound()
{
	if (!isTamed())
		return "";
	if (isInLove())
		return "mob.cat.purr";
	return rand.nextInt(4) == 0 ? "mob.cat.purreow" : "mob.cat.meow";
}

jstring EntityOcelot::getHurtSound()
{
	return "mob.cat.hitt";
}

jstring EntityOcelot::getDeathSound()
{
	return "mob.cat.hitt";
}

float EntityOcelot::getSoundVolume()
{
	return 0.4f;
}

int_t EntityOcelot::getDropItemId()
{
	return Item::leather != nullptr ? Item::leather->shiftedIndex : -1;
}

void EntityOcelot::dropFewItems(bool, int_t lootingLevel)
{
}

bool EntityOcelot::attackEntityAsMob(Entity *target)
{
	if (target == nullptr)
		return false;
	DamageSource source = DamageSource::causeMobDamage(this);
	return target->attackEntityFrom(source, 3);
}

bool EntityOcelot::attackEntityFrom(Entity *source, int_t damage)
{
	if (aiSit != nullptr)
		aiSit->setSitting(false);
	return EntityTameable::attackEntityFrom(source, damage);
}

bool EntityOcelot::attackEntityFrom(const DamageSource &source, int_t damage)
{
	if (aiSit != nullptr)
		aiSit->setSitting(false);
	return EntityTameable::attackEntityFrom(source, damage);
}

bool EntityOcelot::interact(EntityPlayer *player)
{
	if (player == nullptr || player->inventory == nullptr)
		return false;
	ItemStack *stack = player->inventory->getCurrentItem();
	if (!isTamed())
	{
		if (aiTempt != nullptr && aiTempt->isRunning() && stack != nullptr && Item::fishRaw != nullptr &&
			stack->itemID == Item::fishRaw->shiftedIndex && player->getDistanceSqToEntity(this) < 9.0)
		{
			--stack->stackSize;
			if (stack->stackSize <= 0)
			{
				int_t slot = player->inventory->currentItem;
				player->inventory->setInventorySlotContents(slot, nullptr);
				stack = nullptr;
			}
			if (!worldObj->multiplayerWorld)
			{
				if (rand.nextInt(3) == 0)
				{
					setTamed(true);
					setCatType(1 + worldObj->rand.nextInt(3));
					setOwner(player->username);
					showHeartsOrSmokeFX(true);
					if (aiSit != nullptr)
						aiSit->setSitting(true);
					worldObj->setEntityState(this, (byte_t)7);
				}
				else
				{
					showHeartsOrSmokeFX(false);
					worldObj->setEntityState(this, (byte_t)6);
				}
			}
		}
		return true;
	}

	if (String::equalsIgnoreCaseJava(player->username, getOwnerName()) && !worldObj->multiplayerWorld && !isWheat(stack))
	{
		if (aiSit != nullptr)
			aiSit->setSitting(!isSitting());
	}
	return EntityTameable::interact(player);
}

EntityAnimal *EntityOcelot::spawnBabyAnimal(EntityAnimal *)
{
	EntityOcelot *child = new EntityOcelot(worldObj);
	if (isTamed())
	{
		child->setOwner(getOwnerName());
		child->setTamed(true);
		child->setCatType(getCatType());
	}
	return child;
}

bool EntityOcelot::isWheat(ItemStack *itemstack) const
{
	return itemstack != nullptr && Item::fishRaw != nullptr && itemstack->itemID == Item::fishRaw->shiftedIndex;
}

bool EntityOcelot::canMateWith(EntityAnimal *mate) const
{
	if (mate == this || !isTamed())
		return false;
	EntityOcelot *ocelot = dynamic_cast<EntityOcelot *>(mate);
	return ocelot != nullptr && ocelot->isTamed() && isInLove() && ocelot->isInLove();
}

int_t EntityOcelot::getCatType() const
{
	return (int_t)dataWatcher->getWatchableObjectByte(18);
}

void EntityOcelot::setCatType(int_t type)
{
	dataWatcher->updateObject(18, (byte_t)type);
}

bool EntityOcelot::getCanSpawnHere()
{
	if (worldObj == nullptr || worldObj->rand.nextInt(3) == 0 || boundingBox == nullptr)
		return false;
	if (!worldObj->checkIfAABBIsClear(boundingBox) || !worldObj->getCollidingBoundingBoxes(this, boundingBox).empty() ||
		worldObj->getIsAnyLiquid(boundingBox))
		return false;
	int_t x = MathHelper::floor_double(posX);
	int_t y = MathHelper::floor_double(boundingBox->minY);
	int_t z = MathHelper::floor_double(posZ);
	if (y < 63)
		return false;
	int_t blockId = worldObj->getBlockId(x, y - 1, z);
	return (Block::grass != nullptr && blockId == Block::grass->blockID) ||
		(Block::leaves != nullptr && blockId == Block::leaves->blockID);
}
