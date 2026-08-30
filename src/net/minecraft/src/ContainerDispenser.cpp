#include "ContainerDispenser.h"

#include "IInventory.h"
#include "Slot.h"
#include "TileEntityDispenser.h"
#include "EntityPlayer.h"
#include "ItemStack.h"

ContainerDispenser::ContainerDispenser(IInventory *iinventory, TileEntityDispenser *tileentitydispenser)
{
	dispenser = tileentitydispenser;
	// Network-opened dispensers are synthetic and have no World owner.
	ownsDispenser = tileentitydispenser != nullptr && tileentitydispenser->worldObj == nullptr;
	for(int_t i = 0; i < 3; i++)
	{
		for(int_t l = 0; l < 3; l++)
		{
			addSlot(new Slot(tileentitydispenser, l + i * 3, 62 + l * 18, 17 + i * 18));
		}
	}
	for(int_t j = 0; j < 3; j++)
	{
		for(int_t i1 = 0; i1 < 9; i1++)
		{
			addSlot(new Slot(iinventory, i1 + j * 9 + 9, 8 + i1 * 18, 84 + j * 18));
		}
	}
	for(int_t k = 0; k < 9; k++)
	{
		addSlot(new Slot(iinventory, k, 8 + k * 18, 142));
	}
}

ContainerDispenser::~ContainerDispenser()
{
	if (ownsDispenser)
		delete dispenser;
}

bool ContainerDispenser::isUsableByPlayer(EntityPlayer *entityplayer)
{
	return dispenser->canInteractWith(entityplayer);
}


ItemStack *ContainerDispenser::getStackInSlot(int_t slotIndex)
{
	if(slotIndex < 0 || slotIndex >= static_cast<int_t>(slots.size()))
		return nullptr;
	Slot *slot = slots[slotIndex];
	if(slot == nullptr || !slot->getHasStack())
		return nullptr;

	ItemStack *source = slot->getStack();
	ItemStack *original = source->copy();
	const bool moved = slotIndex < 9
		? mergeItemStack(source, 9, 45, true)
		: mergeItemStack(source, 0, 9, false);
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
