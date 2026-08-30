#pragma once

#include <string>

#include "IInventory.h"
#include "java/Type.h"

class Container;
class EntityPlayer;
class ItemStack;

// net.minecraft.src.InventoryCrafting
class InventoryCrafting : public IInventory
{
public:
	InventoryCrafting(Container *container, int_t i, int_t j);
	~InventoryCrafting() override;

	int_t getSizeInventory() override;
	ItemStack *getStackInSlot(int_t i) override;
	ItemStack *getStackInSlotOnClosing(int_t i) override;
	// Java: func_21103_b(row, col) — readable name.
	ItemStack *getStackInRowAndColumn(int_t i, int_t j);
	std::string getInvName() override;
	ItemStack *decrStackSize(int_t i, int_t j) override;
	void setInventorySlotContents(int_t i, ItemStack *itemstack) override;
	int_t getInventoryStackLimit() override;
	void onInventoryChanged() override;
	bool canInteractWith(EntityPlayer *entityplayer) override;

	// ShapedRecipes is the only legacy caller for `getStackAt`; alias for clarity.
	ItemStack *getStackAt(int_t i, int_t j) { return getStackInRowAndColumn(i, j); }

private:
	ItemStack **stackList;
	int_t stackListSize;
	int_t inventoryWidth;
	Container *eventHandler;
};
