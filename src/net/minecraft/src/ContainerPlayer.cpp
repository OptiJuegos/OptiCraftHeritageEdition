#include "ContainerPlayer.h"

#include "InventoryCrafting.h"
#include "InventoryCraftResult.h"
#include "SlotCrafting.h"
#include "InventoryPlayer.h"
#include "Slot.h"
#include "SlotArmor.h"
#include "CraftingManager.h"
#include "IInventory.h"
#include "EntityPlayer.h"
#include "ItemStack.h"

ContainerPlayer::ContainerPlayer(InventoryPlayer *inventoryplayer) :
	ContainerPlayer(inventoryplayer, true)
{
}

ContainerPlayer::ContainerPlayer(InventoryPlayer *inventoryplayer, bool_t flag)
{
	craftMatrix = new InventoryCrafting(this, 2, 2);
	craftResult = new InventoryCraftResult();
	isSinglePlayer = false;
	isSinglePlayer = flag;
	addSlot(new SlotCrafting(inventoryplayer->player, craftMatrix, craftResult, 0, 144, 36));
	for(int_t i = 0; i < 2; i++)
	{
		for(int_t i1 = 0; i1 < 2; i1++)
		{
			addSlot(new Slot(craftMatrix, i1 + i * 2, 88 + i1 * 18, 26 + i * 18));
		}
	}
	for(int_t j = 0; j < 4; j++)
	{
		int_t j1 = j;
		addSlot(new SlotArmor(this, inventoryplayer, inventoryplayer->getSizeInventory() - 1 - j, 8, 8 + j * 18, j1));
	}
	for(int_t k = 0; k < 3; k++)
	{
		for(int_t k1 = 0; k1 < 9; k1++)
		{
			addSlot(new Slot(inventoryplayer, k1 + (k + 1) * 9, 8 + k1 * 18, 84 + k * 18));
		}
	}
	for(int_t l = 0; l < 9; l++)
	{
		addSlot(new Slot(inventoryplayer, l, 8 + l * 18, 142));
	}
	onCraftMatrixChanged(craftMatrix);
}

ContainerPlayer::~ContainerPlayer()
{
	delete craftMatrix;
	delete craftResult;
}

void ContainerPlayer::onCraftMatrixChanged(IInventory *iinventory)
{
	ItemStack *newResult = CraftingManager::getInstance()->findMatchingRecipe(craftMatrix);
	craftResult->setInventorySlotContents(0, newResult);
}

void ContainerPlayer::onCraftGuiClosed(EntityPlayer *entityplayer)
{
	Container::onCraftGuiClosed(entityplayer);
	for(int_t i = 0; i < 4; i++)
	{
		ItemStack *itemstack = craftMatrix->getStackInSlot(i);
		if(itemstack != nullptr)
		{
			entityplayer->dropPlayerItem(itemstack);
			craftMatrix->setInventorySlotContents(i, nullptr);
		}
	}
}

bool ContainerPlayer::isUsableByPlayer(EntityPlayer *entityplayer)
{
	return true;
}

ItemStack *ContainerPlayer::getStackInSlot(int_t slotIndex)
{
	if(slotIndex < 0 || slotIndex >= static_cast<int_t>(slots.size()))
		return nullptr;
	Slot *slot = slots[slotIndex];
	if(slot == nullptr || !slot->getHasStack())
		return nullptr;

	ItemStack *source = slot->getStack();
	ItemStack *original = source->copy();
	bool moved = false;
	if(slotIndex == 0)
	{
		moved = mergeItemStack(source, 9, 45, true);
		if(moved)
			slot->onSlotChange(source, original);
	}
	else if(slotIndex >= 9 && slotIndex < 36)
		moved = mergeItemStack(source, 36, 45, false);
	else if(slotIndex >= 36 && slotIndex < 45)
		moved = mergeItemStack(source, 9, 36, false);
	else
		moved = mergeItemStack(source, 9, 45, false);

	if(!moved)
	{
		delete original;
		return nullptr;
	}

	ItemStack *orphaned = nullptr;
	if(source->stackSize == 0)
	{
		orphaned = slot->takeStack();
	}
	else
		slot->onSlotChanged();

	if(source->stackSize == original->stackSize)
	{
		delete orphaned;
		delete original;
		return nullptr;
	}

	slot->onPickupFromSlot(source);
	delete orphaned;
	return original;
}
