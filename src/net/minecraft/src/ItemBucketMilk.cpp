#include "ItemBucketMilk.h"

#include "EntityPlayer.h"
#include "ItemStack.h"
#include "World.h"

ItemBucketMilk::ItemBucketMilk(int id) : Item(id)
{
	maxStackSize = 1;
}

ItemStack *ItemBucketMilk::onFoodEaten(ItemStack *itemstack, World *world, EntityPlayer *entityplayer)
{
	if (itemstack == nullptr)
		return nullptr;
	--itemstack->stackSize;
	if (world != nullptr && entityplayer != nullptr && !world->multiplayerWorld)
		entityplayer->clearActivePotions();
	return itemstack->stackSize <= 0 ? new ItemStack(Item::bucketEmpty) : itemstack;
}

int ItemBucketMilk::getMaxItemUseDuration(ItemStack *)
{
	return 32;
}

EnumAction ItemBucketMilk::getItemUseAction(ItemStack *)
{
	return EnumAction::drink;
}

ItemStack *ItemBucketMilk::onItemRightClick(ItemStack *itemstack, World *, EntityPlayer *entityplayer)
{
	if (entityplayer != nullptr)
		entityplayer->setItemInUse(itemstack, getMaxItemUseDuration(itemstack));
	return itemstack;
}
