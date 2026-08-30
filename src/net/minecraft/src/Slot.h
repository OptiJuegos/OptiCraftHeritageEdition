#pragma once

#include "java/Type.h"

class IInventory;
class ItemStack;

// net.minecraft.src.Slot
class Slot
{
public:
	Slot(IInventory *iinventory, int_t i, int_t j, int_t k);
	virtual ~Slot() = default;

	void onSlotChange(ItemStack *currentStack, ItemStack *originalStack);
	virtual void onPickupFromSlot(ItemStack *itemstack);
	virtual bool isItemValid(ItemStack *itemstack);
	ItemStack *getStack();
	bool getHasStack();
	void putStack(ItemStack *itemstack);
	ItemStack *takeStack();
	virtual void onSlotChanged();
	virtual int_t getSlotStackLimit();
	virtual int_t getBackgroundIconIndex();
	virtual ItemStack *decrStackSize(int_t i);

	// Not in Java: Container::usesInventory() needs to know which IInventory a
	// slot is backed by so an open container can be closed before that inventory
	// (a TileEntity) is deleted. Java never had to ask -- the GC kept the object
	// alive for as long as the Slot referenced it.
	IInventory *getInventory() const { return inventory; }

protected:
	virtual void onCrafting(ItemStack *itemstack, int_t amount);
	virtual void onCrafting(ItemStack *itemstack);

private:
	const int_t slotIndex;
	IInventory * const inventory;

public:
	int_t slotNumber = 0;
	int_t xDisplayPosition;
	int_t yDisplayPosition;
};
