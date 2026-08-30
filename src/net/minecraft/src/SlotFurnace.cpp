#include "SlotFurnace.h"

#include "EntityPlayer.h"
#include "IInventory.h"
#include "ItemStack.h"
#include "Item.h"
#include "AchievementList.h"

#include <algorithm>

SlotFurnace::SlotFurnace(EntityPlayer *entityplayer, IInventory *iinventory, int_t i, int_t j, int_t k) :
	Slot(iinventory, i, j, k),
	thePlayer(entityplayer)
{
}

bool SlotFurnace::isItemValid(ItemStack *itemstack)
{
	return false;
}

ItemStack *SlotFurnace::decrStackSize(int_t amount)
{
	if(getHasStack())
		removeCount += std::min(amount, getStack()->stackSize);
	return Slot::decrStackSize(amount);
}

void SlotFurnace::onCrafting(ItemStack *itemstack, int_t amount)
{
	removeCount += amount;
	onCrafting(itemstack);
}

void SlotFurnace::onCrafting(ItemStack *itemstack)
{
	itemstack->onCrafting(thePlayer->worldObj, thePlayer, removeCount);
	removeCount = 0;
	if(itemstack->itemID == Item::ingotIron->shiftedIndex)
	{
		thePlayer->addStat(AchievementList::acquireIron, 1);
	}
	if(itemstack->itemID == Item::fishCooked->shiftedIndex)
	{
		thePlayer->addStat(AchievementList::cookFish, 1);
	}
}

void SlotFurnace::onPickupFromSlot(ItemStack *itemstack)
{
	onCrafting(itemstack);
	Slot::onPickupFromSlot(itemstack);
}
