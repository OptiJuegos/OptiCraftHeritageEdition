#include "ContainerChest.h"

#include "IInventory.h"
#include "Slot.h"
#include "ItemStack.h"
#include "EntityPlayer.h"

ContainerChest::ContainerChest(IInventory *iinventory, IInventory *iinventory1, bool ownsLowerChestInventory)
	: lowerChestInventory(iinventory1)
	, ownsLowerChestInventory(ownsLowerChestInventory)
{
	numRows = iinventory1->getSizeInventory() / 9;
	lowerChestInventory->openChest();
	int_t i = (numRows - 4) * 18;
	for(int_t j = 0; j < numRows; j++)
	{
		for(int_t i1 = 0; i1 < 9; i1++)
		{
			addSlot(new Slot(iinventory1, i1 + j * 9, 8 + i1 * 18, 18 + j * 18));
		}
	}
	for(int_t k = 0; k < 3; k++)
	{
		for(int_t j1 = 0; j1 < 9; j1++)
		{
			addSlot(new Slot(iinventory, j1 + k * 9 + 9, 8 + j1 * 18, 103 + k * 18 + i));
		}
	}
	for(int_t l = 0; l < 9; l++)
	{
		addSlot(new Slot(iinventory, l, 8 + l * 18, 161 + i));
	}
}

ContainerChest::~ContainerChest()
{
	if (ownsLowerChestInventory)
		delete lowerChestInventory;
}

bool ContainerChest::isUsableByPlayer(EntityPlayer *entityplayer)
{
	return lowerChestInventory->canInteractWith(entityplayer);
}

ItemStack *ContainerChest::getStackInSlot(int_t slotIndex)
{
	if(slotIndex < 0 || slotIndex >= static_cast<int_t>(slots.size()))
		return nullptr;
	Slot *slot = slots[slotIndex];
	if(slot == nullptr || !slot->getHasStack())
		return nullptr;

	ItemStack *source = slot->getStack();
	ItemStack *original = source->copy();
	const bool moved = slotIndex < numRows * 9
		? mergeItemStack(source, numRows * 9, static_cast<int_t>(slots.size()), true)
		: mergeItemStack(source, 0, numRows * 9, false);
	if(!moved)
	{
		delete original;
		return nullptr;
	}

	if(source->stackSize == 0)
	{
		delete slot->takeStack();
	}
	else
	{
		slot->onSlotChanged();
	}
	return original;
}

void ContainerChest::onCraftGuiClosed(EntityPlayer *entityplayer)
{
	Container::onCraftGuiClosed(entityplayer);
	lowerChestInventory->closeChest();
}
