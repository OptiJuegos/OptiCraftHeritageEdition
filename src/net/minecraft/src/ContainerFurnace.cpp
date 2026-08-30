#include "ContainerFurnace.h"

#include "Slot.h"
#include "SlotFurnace.h"
#include "InventoryPlayer.h"
#include "ICrafting.h"
#include "TileEntityFurnace.h"
#include "ItemStack.h"
#include "EntityPlayer.h"
#include "FurnaceRecipes.h"
#include "Item.h"

ContainerFurnace::ContainerFurnace(InventoryPlayer *inventoryplayer, TileEntityFurnace *tileentityfurnace)
{
	cookTime = 0;
	burnTime = 0;
	itemBurnTime = 0;
	furnace = tileentityfurnace;
	// Multiplayer creates a synthetic TileEntityFurnace with no World.
	// A real world tile is borrowed; the synthetic one belongs to this container.
	ownsFurnace = tileentityfurnace != nullptr && tileentityfurnace->worldObj == nullptr;
	addSlot(new Slot(tileentityfurnace, 0, 56, 17));
	addSlot(new Slot(tileentityfurnace, 1, 56, 53));
	addSlot(new SlotFurnace(inventoryplayer->player, tileentityfurnace, 2, 116, 35));
	for(int_t i = 0; i < 3; i++)
	{
		for(int_t k = 0; k < 9; k++)
		{
			addSlot(new Slot(inventoryplayer, k + i * 9 + 9, 8 + k * 18, 84 + i * 18));
		}
	}
	for(int_t j = 0; j < 9; j++)
	{
		addSlot(new Slot(inventoryplayer, j, 8 + j * 18, 142));
	}
}

ContainerFurnace::~ContainerFurnace()
{
	if (ownsFurnace)
		delete furnace;
}

void ContainerFurnace::updateCraftingResults()
{
	Container::updateCraftingResults();
	for(size_t i = 0; i < crafters.size(); i++)
	{
		ICrafting *icrafting = crafters[i];
		if(cookTime != furnace->furnaceCookTime)
		{
			icrafting->sendProgressBarUpdate(this, 0, furnace->furnaceCookTime);
		}
		if(burnTime != furnace->furnaceBurnTime)
		{
			icrafting->sendProgressBarUpdate(this, 1, furnace->furnaceBurnTime);
		}
		if(itemBurnTime != furnace->currentItemBurnTime)
		{
			icrafting->sendProgressBarUpdate(this, 2, furnace->currentItemBurnTime);
		}
	}
	cookTime = furnace->furnaceCookTime;
	burnTime = furnace->furnaceBurnTime;
	itemBurnTime = furnace->currentItemBurnTime;
}

void ContainerFurnace::updateProgressBar(int_t id, int_t value)
{
	if(id == 0)
	{
		furnace->furnaceCookTime = value;
	}
	if(id == 1)
	{
		furnace->furnaceBurnTime = value;
	}
	if(id == 2)
	{
		furnace->currentItemBurnTime = value;
	}
}

bool ContainerFurnace::isUsableByPlayer(EntityPlayer *entityplayer)
{
	return furnace->canInteractWith(entityplayer);
}

ItemStack *ContainerFurnace::getStackInSlot(int_t slotIndex)
{
	if(slotIndex < 0 || slotIndex >= static_cast<int_t>(slots.size()))
		return nullptr;

	Slot *slot = slots[slotIndex];
	if(slot == nullptr || !slot->getHasStack())
		return nullptr;

	ItemStack *source = slot->getStack();
	ItemStack *original = source->copy();
	bool moved = false;

	if(slotIndex == 2)
	{
		moved = mergeItemStack(source, 3, 39, true);
		if(moved)
			slot->onSlotChange(source, original);
	}
	else if(slotIndex != 1 && slotIndex != 0)
	{
		Item *item = source->getItem();
		if(item != nullptr && FurnaceRecipes::smelting()->getSmeltingResult(item->shiftedIndex) != nullptr)
			moved = mergeItemStack(source, 0, 1, false);
		else if(TileEntityFurnace::isItemFuel(source))
			moved = mergeItemStack(source, 1, 2, false);
		else if(slotIndex >= 3 && slotIndex < 30)
			moved = mergeItemStack(source, 30, 39, false);
		else if(slotIndex >= 30 && slotIndex < 39)
			moved = mergeItemStack(source, 3, 30, false);
	}
	else
	{
		moved = mergeItemStack(source, 3, 39, false);
	}

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
	{
		slot->onSlotChanged();
	}

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
