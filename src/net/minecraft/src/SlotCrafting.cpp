#include "SlotCrafting.h"

#include "EntityPlayer.h"
#include "InventoryPlayer.h"
#include "IInventory.h"
#include "ItemStack.h"
#include "Block.h"
#include "Item.h"
#include "AchievementList.h"

#include <algorithm>

SlotCrafting::SlotCrafting(EntityPlayer *entityplayer, IInventory *iinventory, IInventory *iinventory1, int_t i, int_t j, int_t k) :
	Slot(iinventory1, i, j, k),
	thePlayer(entityplayer),
	craftMatrix(iinventory)
{
}

bool SlotCrafting::isItemValid(ItemStack *itemstack)
{
	return false;
}

ItemStack *SlotCrafting::decrStackSize(int_t amount)
{
	if(getHasStack())
		removeCount += std::min(amount, getStack()->stackSize);
	return Slot::decrStackSize(amount);
}

void SlotCrafting::onCrafting(ItemStack *itemstack, int_t amount)
{
	removeCount += amount;
	onCrafting(itemstack);
}

void SlotCrafting::onCrafting(ItemStack *itemstack)
{
	itemstack->onCrafting(thePlayer->worldObj, thePlayer, removeCount);
	removeCount = 0;
	if(itemstack->itemID == Block::workbench->blockID)
	{
		thePlayer->addStat(AchievementList::buildWorkBench, 1);
	}
	else if(itemstack->itemID == Item::pickaxeWood->shiftedIndex)
	{
		thePlayer->addStat(AchievementList::buildPickaxe, 1);
	}
	else if(itemstack->itemID == Block::stoneOvenIdle->blockID)
	{
		thePlayer->addStat(AchievementList::buildFurnace, 1);
	}
	else if(itemstack->itemID == Item::hoeWood->shiftedIndex)
	{
		thePlayer->addStat(AchievementList::buildHoe, 1);
	}
	else if(itemstack->itemID == Item::bread->shiftedIndex)
	{
		thePlayer->addStat(AchievementList::makeBread, 1);
	}
	else if(itemstack->itemID == Item::cake->shiftedIndex)
	{
		thePlayer->addStat(AchievementList::bakeCake, 1);
	}
	else if(itemstack->itemID == Item::pickaxeStone->shiftedIndex)
	{
		thePlayer->addStat(AchievementList::buildBetterPickaxe, 1);
	}
	else if(itemstack->itemID == Item::swordWood->shiftedIndex)
	{
		thePlayer->addStat(AchievementList::buildSword, 1);
	}
	else if(itemstack->itemID == Block::enchantmentTable->blockID)
	{
		thePlayer->addStat(AchievementList::enchantments, 1);
	}
	else if(itemstack->itemID == Block::bookShelf->blockID)
	{
		thePlayer->addStat(AchievementList::bookcase, 1);
	}
}

void SlotCrafting::onPickupFromSlot(ItemStack *itemstack)
{
	onCrafting(itemstack);

	for(int_t i = 0; i < craftMatrix->getSizeInventory(); ++i)
	{
		ItemStack *ingredient = craftMatrix->getStackInSlot(i);
		if(ingredient == nullptr)
			continue;

		Item *item = ingredient->getItem();
		const bool leavesCraftingGrid = item == nullptr || item->doesContainerItemLeaveCraftingGrid(ingredient);
		ItemStack *consumed = craftMatrix->decrStackSize(i, 1);

		if(item != nullptr && item->hasContainerItem())
		{
			ItemStack *container = new ItemStack(item->getContainerItem());
			if(!leavesCraftingGrid ||
			   !thePlayer->inventory->addItemStackToInventory(container))
			{
				if(craftMatrix->getStackInSlot(i) == nullptr)
				{
					craftMatrix->setInventorySlotContents(i, container);
					container = nullptr;
				}
				else
				{
					thePlayer->dropPlayerItem(container);
					container = nullptr;
				}
			}
			delete container;
		}

		delete consumed;
	}
}
