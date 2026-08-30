#pragma once

#include <string>

#include "java/Type.h"

class EntityPlayer;
class ItemStack;

// net.minecraft.src.IInventory
class IInventory
{
public:
	virtual ~IInventory() = default;

	virtual int_t getSizeInventory() = 0;
	virtual ItemStack *getStackInSlot(int_t i) = 0;
	virtual ItemStack *getStackInSlotOnClosing(int_t i) = 0;
	virtual ItemStack *decrStackSize(int_t i, int_t j) = 0;
	virtual void setInventorySlotContents(int_t i, ItemStack *itemstack) = 0;
	virtual std::string getInvName() = 0;
	virtual int_t getInventoryStackLimit() = 0;
	virtual void onInventoryChanged() = 0;
	virtual bool canInteractWith(EntityPlayer *entityplayer) = 0;
	virtual void openChest() {}
	virtual void closeChest() {}
};
