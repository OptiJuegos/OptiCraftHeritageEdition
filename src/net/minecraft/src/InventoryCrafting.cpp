#include "InventoryCrafting.h"

#include "Container.h"
#include "ItemStack.h"

InventoryCrafting::InventoryCrafting(Container *container, int_t i, int_t j)
{
	int_t k = i * j;
	stackList = new ItemStack *[k]();
	for (int_t s = 0; s < k; s++)
		stackList[s] = nullptr;
	stackListSize = k;
	eventHandler = container;
	inventoryWidth = i;
}

InventoryCrafting::~InventoryCrafting()
{
	for (int_t i = 0; i < stackListSize; ++i)
		delete stackList[i];
	delete[] stackList;
}

int_t InventoryCrafting::getSizeInventory()
{
	return stackListSize;
}

ItemStack *InventoryCrafting::getStackInSlot(int_t i)
{
	if (i < 0 || i >= getSizeInventory())
		return nullptr;
	return stackList[i];
}

ItemStack *InventoryCrafting::getStackInRowAndColumn(int_t i, int_t j)
{
	if (i < 0 || i >= inventoryWidth)
		return nullptr;
	return getStackInSlot(i + j * inventoryWidth);
}

std::string InventoryCrafting::getInvName()
{
	return "container.crafting";
}


ItemStack *InventoryCrafting::getStackInSlotOnClosing(int_t i)
{
	if (i < 0 || i >= getSizeInventory() || stackList[i] == nullptr)
		return nullptr;
	ItemStack *stack = stackList[i];
	stackList[i] = nullptr;
	return stack;
}

ItemStack *InventoryCrafting::decrStackSize(int_t i, int_t j)
{
	if (i < 0 || i >= getSizeInventory() || j <= 0 || stackList[i] == nullptr)
		return nullptr;
	if (stackList[i]->stackSize <= j)
	{
		ItemStack *itemstack = stackList[i];
		stackList[i] = nullptr;
		eventHandler->onCraftMatrixChanged(this);
		return itemstack;
	}
	ItemStack *itemstack1 = stackList[i]->splitStack(j);
	if (stackList[i]->stackSize == 0)
	{
		delete stackList[i];
		stackList[i] = nullptr;
	}
	eventHandler->onCraftMatrixChanged(this);
	return itemstack1;
}

void InventoryCrafting::setInventorySlotContents(int_t i, ItemStack *itemstack)
{
	if (i < 0 || i >= getSizeInventory())
	{
		delete itemstack;
		return;
	}
	if (stackList[i] != itemstack)
		delete stackList[i];
	stackList[i] = itemstack;
	eventHandler->onCraftMatrixChanged(this);
}

int_t InventoryCrafting::getInventoryStackLimit()
{
	return 64;
}

void InventoryCrafting::onInventoryChanged()
{
}

bool InventoryCrafting::canInteractWith(EntityPlayer *entityplayer)
{
	return true;
}
