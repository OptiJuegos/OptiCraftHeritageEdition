#include "ContainerWorkbench.h"

#include "InventoryCrafting.h"
#include "InventoryCraftResult.h"
#include "SlotCrafting.h"
#include "InventoryPlayer.h"
#include "Slot.h"
#include "CraftingManager.h"
#include "IInventory.h"
#include "World.h"
#include "EntityPlayer.h"
#include "Block.h"
#include "ItemStack.h"

ContainerWorkbench::ContainerWorkbench(InventoryPlayer *inventoryplayer, World *world, int_t i, int_t j, int_t k)
{
	craftMatrix = new InventoryCrafting(this, 3, 3);
	craftResult = new InventoryCraftResult();
	this->world = world;
	posX = i;
	posY = j;
	posZ = k;
	addSlot(new SlotCrafting(inventoryplayer->player, craftMatrix, craftResult, 0, 124, 35));
	for(int_t l = 0; l < 3; l++)
	{
		for(int_t k1 = 0; k1 < 3; k1++)
		{
			addSlot(new Slot(craftMatrix, k1 + l * 3, 30 + k1 * 18, 17 + l * 18));
		}
	}
	for(int_t i1 = 0; i1 < 3; i1++)
	{
		for(int_t l1 = 0; l1 < 9; l1++)
		{
			addSlot(new Slot(inventoryplayer, l1 + i1 * 9 + 9, 8 + l1 * 18, 84 + i1 * 18));
		}
	}
	for(int_t j1 = 0; j1 < 9; j1++)
	{
		addSlot(new Slot(inventoryplayer, j1, 8 + j1 * 18, 142));
	}
	onCraftMatrixChanged(craftMatrix);
}

ContainerWorkbench::~ContainerWorkbench()
{
	delete craftMatrix;
	delete craftResult;
}

void ContainerWorkbench::onCraftMatrixChanged(IInventory *iinventory)
{
	ItemStack *newResult = CraftingManager::getInstance()->findMatchingRecipe(craftMatrix);
	craftResult->setInventorySlotContents(0, newResult);
}

void ContainerWorkbench::onCraftGuiClosed(EntityPlayer *entityplayer)
{
	Container::onCraftGuiClosed(entityplayer);
	if(world->multiplayerWorld)
	{
		return;
	}
	for(int_t i = 0; i < 9; i++)
	{
		ItemStack *itemstack = craftMatrix->getStackInSlot(i);
		if(itemstack != nullptr)
		{
			entityplayer->dropPlayerItem(itemstack);
			craftMatrix->setInventorySlotContents(i, nullptr);
		}
	}
}

bool ContainerWorkbench::isUsableByPlayer(EntityPlayer *entityplayer)
{
	if(world->getBlockId(posX, posY, posZ) != Block::workbench->blockID)
	{
		return false;
	}
	return entityplayer->getDistanceSq((double)posX + 0.5, (double)posY + 0.5, (double)posZ + 0.5) <= 64.0;
}

ItemStack *ContainerWorkbench::getStackInSlot(int_t slotIndex)
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
		moved = mergeItemStack(source, 10, 46, true);
		if(moved)
			slot->onSlotChange(source, original);
	}
	else if(slotIndex >= 10 && slotIndex < 37)
		moved = mergeItemStack(source, 37, 46, false);
	else if(slotIndex >= 37 && slotIndex < 46)
		moved = mergeItemStack(source, 10, 37, false);
	else
		moved = mergeItemStack(source, 10, 46, false);

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
