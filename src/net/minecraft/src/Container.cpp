#include "Container.h"

#include "Slot.h"
#include "ItemStack.h"
#include "ICrafting.h"
#include "EntityPlayer.h"
#include "InventoryPlayer.h"
#include "IInventory.h"

Container::Container()
{
	inventoryItemStacks.clear();
	slots.clear();
	windowId = 0;
	transactionID = 0;
	crafters.clear();
	playerList.clear();
}

Container::~Container()
{
	for (Slot *slot : slots)
		delete slot;
	for (ItemStack *stack : inventoryItemStacks)
		delete stack;
}

void Container::addSlot(Slot *slot)
{
	slot->slotNumber = slots.size();
	slots.push_back(slot);
	inventoryItemStacks.push_back(nullptr);
}

void Container::updateCraftingResults()
{
	for(size_t i = 0; i < slots.size(); i++)
	{
		ItemStack *itemstack = slots[i]->getStack();
		ItemStack *itemstack1 = inventoryItemStacks[i];
		if(ItemStack::areItemStacksEqual(itemstack1, itemstack))
		{
			continue;
		}
		delete itemstack1;
		itemstack1 = itemstack != nullptr ? itemstack->copy() : nullptr;
		inventoryItemStacks[i] = itemstack1;
		for(size_t j = 0; j < crafters.size(); j++)
		{
			crafters[j]->sendSlotContents(this, i, itemstack1);
		}
	}
}

bool Container::enchantItem(EntityPlayer *, int_t)
{
	return false;
}

Slot *Container::getSlot(int_t i)
{
	return i >= 0 && (std::size_t)i < slots.size() ? slots[(std::size_t)i] : nullptr;
}

bool Container::usesInventory(IInventory *iinventory)
{
	if (iinventory == nullptr)
		return false;
	for (Slot *slot : slots)
	{
		if (slot != nullptr && slot->getInventory() == iinventory)
			return true;
	}
	return false;
}

ItemStack *Container::getStackInSlot(int_t i)
{
	Slot *slot = slots[i];
	ItemStack *stack = slot != nullptr ? slot->getStack() : nullptr;
	return stack != nullptr ? stack->copy() : nullptr;
}

ItemStack *Container::slotClick(int_t slotIndex, int_t mouseButton, bool_t shiftClick, EntityPlayer *entityplayer)
{
	ItemStack *itemstack = nullptr;
	if(mouseButton == 0 || mouseButton == 1)
	{
		InventoryPlayer *inventoryplayer = entityplayer->inventory;
		if(slotIndex == -999)
		{
			if(inventoryplayer->getItemStack() != nullptr && slotIndex == -999)
			{
				if(mouseButton == 0)
				{
					entityplayer->dropPlayerItem(inventoryplayer->getItemStack());
					inventoryplayer->setItemStack(nullptr);
				}
				if(mouseButton == 1)
				{
					entityplayer->dropPlayerItem(inventoryplayer->getItemStack()->splitStack(1));
					if(inventoryplayer->getItemStack()->stackSize == 0)
					{
						// Java dropped the reference and let the GC take the
						// emptied stack. Here nothing else owns it.
						ItemStack *emptied = inventoryplayer->getItemStack();
						inventoryplayer->setItemStack(nullptr);
						delete emptied;
					}
				}
			}
		}
		else if(shiftClick)
		{
			ItemStack *transferred = getStackInSlot(slotIndex);
			if(transferred != nullptr)
			{
				int_t itemId = transferred->itemID;
				itemstack = transferred;
				Slot *slot = slots[slotIndex];
				if(slot != nullptr && slot->getStack() != nullptr && slot->getStack()->itemID == itemId)
					retrySlotClick(slotIndex, mouseButton, shiftClick, entityplayer);
			}
		}
		else
		{
			Slot *slot = slots[slotIndex];
			if(slot != nullptr)
			{
				slot->onSlotChanged();
				ItemStack *itemstack2 = slot->getStack();
				ItemStack *itemstack3 = inventoryplayer->getItemStack();
				if(itemstack2 != nullptr)
				{
					itemstack = itemstack2->copy();
				}
				if(itemstack2 == nullptr)
				{
					if(itemstack3 != nullptr && slot->isItemValid(itemstack3))
					{
						int_t i1 = mouseButton != 0 ? 1 : itemstack3->stackSize;
						if(i1 > slot->getSlotStackLimit())
						{
							i1 = slot->getSlotStackLimit();
						}
						slot->putStack(itemstack3->splitStack(i1));
						if(itemstack3->stackSize == 0)
						{
							// The split went into the slot; what is left in hand is
							// an empty husk that only the GC used to clean up.
							inventoryplayer->setItemStack(nullptr);
							delete itemstack3;
							itemstack3 = nullptr;
						}
					}
				}
				else if(itemstack3 == nullptr)
				{
					int_t j1 = mouseButton != 0 ? (itemstack2->stackSize + 1) / 2 : itemstack2->stackSize;
					ItemStack *itemstack5 = slot->decrStackSize(j1);
					inventoryplayer->setItemStack(itemstack5);
					if(itemstack2->stackSize == 0)
					{
						slot->putStack(nullptr);
					}
					slot->onPickupFromSlot(inventoryplayer->getItemStack());
				}
				else if(slot->isItemValid(itemstack3))
				{
					if(itemstack2->itemID != itemstack3->itemID || itemstack2->getHasSubtypes() && itemstack2->getItemDamage() != itemstack3->getItemDamage() || !ItemStack::func_46154_a(itemstack2, itemstack3))
					{
						if(itemstack3->stackSize <= slot->getSlotStackLimit())
						{
							ItemStack *itemstack4 = slot->takeStack();
							slot->putStack(itemstack3);
							inventoryplayer->setItemStack(itemstack4);
						}
					}
					else
					{
						int_t k1 = mouseButton != 0 ? 1 : itemstack3->stackSize;
						if(k1 > slot->getSlotStackLimit() - itemstack2->stackSize)
						{
							k1 = slot->getSlotStackLimit() - itemstack2->stackSize;
						}
						if(k1 > itemstack3->getMaxStackSize() - itemstack2->stackSize)
						{
							k1 = itemstack3->getMaxStackSize() - itemstack2->stackSize;
						}
						// splitStack() allocates; Java discarded the result because the
						// count is merged into itemstack2 below, so the object is dead
						// on arrival and has to be freed here.
						delete itemstack3->splitStack(k1);
						if(itemstack3->stackSize == 0)
						{
							inventoryplayer->setItemStack(nullptr);
							delete itemstack3;
							itemstack3 = nullptr;
						}
						itemstack2->stackSize += k1;
					}
				}
				else if(itemstack2->itemID == itemstack3->itemID && itemstack3->getMaxStackSize() > 1 && (!itemstack2->getHasSubtypes() || itemstack2->getItemDamage() == itemstack3->getItemDamage()) && ItemStack::func_46154_a(itemstack2, itemstack3))
				{
					int_t l1 = itemstack2->stackSize;
					if(l1 > 0 && l1 + itemstack3->stackSize <= itemstack3->getMaxStackSize())
					{
						itemstack3->stackSize += l1;
						ItemStack *removed = slot->decrStackSize(l1);
						slot->onPickupFromSlot(inventoryplayer->getItemStack());
						delete removed;
					}
				}
			}
		}
	}
	return itemstack;
}

void Container::retrySlotClick(int_t slotIndex, int_t mouseButton, bool shiftClick, EntityPlayer *entityplayer)
{
	ItemStack *result = slotClick(slotIndex, mouseButton, shiftClick, entityplayer);
	delete result;
}

void Container::onCraftGuiClosed(EntityPlayer *entityplayer)
{
	InventoryPlayer *inventoryplayer = entityplayer->inventory;
	if(inventoryplayer->getItemStack() != nullptr)
	{
		entityplayer->dropPlayerItem(inventoryplayer->getItemStack());
		inventoryplayer->setItemStack(nullptr);
	}
}

void Container::onCraftMatrixChanged(IInventory *iinventory)
{
	updateCraftingResults();
}

void Container::putStackInSlot(int_t i, ItemStack *itemstack)
{
	Slot *slot = getSlot(i);
	if (slot == nullptr)
	{
		delete itemstack;
		return;
	}
	slot->putStack(itemstack);
}

void Container::putStacksInSlots(ItemStack **aitemstack, int_t length)
{
	if (aitemstack == nullptr || length <= 0)
		return;
	for(int_t i = 0; i < length; i++)
	{
		Slot *slot = getSlot(i);
		if (slot != nullptr)
			slot->putStack(aitemstack[i]);
		else
			delete aitemstack[i];
	}
}

void Container::updateProgressBar(int_t id, int_t value)
{
}

short_t Container::getNextTransactionID(InventoryPlayer *inventoryplayer)
{
	transactionID++;
	return transactionID;
}

void Container::updateTransaction(short_t transactionID)
{
}

void Container::onTransactionResponse(short_t transactionID)
{
}

bool Container::mergeItemStack(ItemStack *itemstack, int_t startSlot, int_t endSlot, bool_t lastToFirst)
{
	bool changed = false;
	int_t slotIndex = lastToFirst ? endSlot - 1 : startSlot;

	if(itemstack->isStackable())
	{
		while(itemstack->stackSize > 0 && (!lastToFirst && slotIndex < endSlot || lastToFirst && slotIndex >= startSlot))
		{
			Slot *slot = slots[slotIndex];
			ItemStack *slotStack = slot->getStack();
			if(slotStack != nullptr && slotStack->itemID == itemstack->itemID &&
			   (!itemstack->getHasSubtypes() || itemstack->getItemDamage() == slotStack->getItemDamage()) &&
			   ItemStack::func_46154_a(itemstack, slotStack))
			{
				int_t combinedSize = slotStack->stackSize + itemstack->stackSize;
				if(combinedSize <= itemstack->getMaxStackSize())
				{
					itemstack->stackSize = 0;
					slotStack->stackSize = combinedSize;
					slot->onSlotChanged();
					changed = true;
				}
				else if(slotStack->stackSize < itemstack->getMaxStackSize())
				{
					itemstack->stackSize -= itemstack->getMaxStackSize() - slotStack->stackSize;
					slotStack->stackSize = itemstack->getMaxStackSize();
					slot->onSlotChanged();
					changed = true;
				}
			}

			slotIndex += lastToFirst ? -1 : 1;
		}
	}

	if(itemstack->stackSize > 0)
	{
		slotIndex = lastToFirst ? endSlot - 1 : startSlot;
		while(!lastToFirst && slotIndex < endSlot || lastToFirst && slotIndex >= startSlot)
		{
			Slot *slot = slots[slotIndex];
			if(slot->getStack() == nullptr)
			{
				slot->putStack(itemstack->copy());
				slot->onSlotChanged();
				itemstack->stackSize = 0;
				changed = true;
				break;
			}

			slotIndex += lastToFirst ? -1 : 1;
		}
	}

	return changed;
}
