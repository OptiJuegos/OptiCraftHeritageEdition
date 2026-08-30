#include "EntityCow.h"

#include "EntityAIFollowParent.h"
#include "EntityAILookIdle.h"
#include "EntityAIMate.h"
#include "EntityAIPanic.h"
#include "EntityAISwimming.h"
#include "EntityAITempt.h"
#include "EntityAIWander.h"
#include "EntityAIWatchClosest.h"
#include "EntityPlayer.h"
#include "InventoryPlayer.h"
#include "Item.h"
#include "ItemStack.h"
#include "PathNavigate.h"
#include "World.h"

EntityCow::EntityCow(World *world)
	: EntityAnimal(world)
{
	ensureEntityInit();
	texture = "/mob/cow.png";
	setSize(0.9f, 1.3f);
	if (getNavigator() != nullptr)
		getNavigator()->setAvoidsWater(true);
	tasks.addTask(0, new EntityAISwimming(this));
	tasks.addTask(1, new EntityAIPanic(this, 0.38f));
	tasks.addTask(2, new EntityAIMate(this, 0.2f));
	tasks.addTask(3, new EntityAITempt(this, 0.25f, Item::wheat->shiftedIndex, false));
	tasks.addTask(4, new EntityAIFollowParent(this, 0.25f));
	tasks.addTask(5, new EntityAIWander(this, 0.2f));
	tasks.addTask(6, new EntityAIWatchClosest(this, typeid(EntityPlayer), 6.0f));
	tasks.addTask(7, new EntityAILookIdle(this));
	setHealth(getMaxHealth());
}

bool EntityCow::isAIEnabled()
{
	return true;
}

int_t EntityCow::getMaxHealth() const
{
	return 10;
}

jstring EntityCow::getLivingSound() { return "mob.cow"; }
jstring EntityCow::getHurtSound() { return "mob.cowhurt"; }
jstring EntityCow::getDeathSound() { return "mob.cowhurt"; }
float EntityCow::getSoundVolume() { return 0.4f; }
int_t EntityCow::getDropItemId() { return Item::leather->shiftedIndex; }

void EntityCow::dropFewItems(bool, int_t lootingLevel)
{
	const int_t leatherCountBase = rand.nextInt(3);
	const int_t leatherCountLooting = rand.nextInt(1 + lootingLevel);
	const int_t leatherCount = leatherCountBase + leatherCountLooting;
	for (int_t i = 0; i < leatherCount; ++i)
		dropItem(Item::leather->shiftedIndex, 1);

	const int_t beefCountBase = rand.nextInt(3);
	const int_t beefCountLooting = rand.nextInt(1 + lootingLevel);
	const int_t beefCount = beefCountBase + 1 + beefCountLooting;
	Item *meat = isBurning() ? Item::beefCooked : Item::beefRaw;
	if (meat != nullptr)
	{
		for (int_t i = 0; i < beefCount; ++i)
			dropItem(meat->shiftedIndex, 1);
	}
}

bool EntityCow::interact(EntityPlayer *entityplayer)
{
	ItemStack *itemstack = entityplayer != nullptr ? entityplayer->inventory->getCurrentItem() : nullptr;
	if (itemstack != nullptr && itemstack->itemID == Item::bucketEmpty->shiftedIndex)
	{
		const int_t slot = entityplayer->inventory->currentItem;
		entityplayer->inventory->setInventorySlotContents(slot, new ItemStack(Item::bucketMilk));
		return true;
	}
	return EntityAnimal::interact(entityplayer);
}

EntityAnimal *EntityCow::spawnBabyAnimal(EntityAnimal *mate)
{
	(void)mate;
	return new EntityCow(worldObj);
}
