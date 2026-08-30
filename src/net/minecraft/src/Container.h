#pragma once

#include <set>
#include <vector>

#include "java/Type.h"

class EntityPlayer;
class ICrafting;
class IInventory;
class InventoryPlayer;
class ItemStack;
class Slot;

// net.minecraft.src.Container
class Container
{
public:
	Container();
	virtual ~Container();

protected:
	void addSlot(Slot *slot);

public:
	virtual void updateCraftingResults();
	virtual bool enchantItem(EntityPlayer *entityplayer, int_t enchantment);
	Slot *getSlot(int_t i);
	virtual ItemStack *getStackInSlot(int_t i);

	// Java: func_27280_a — handles a click on a slot (left/right + shift-click).
	ItemStack *slotClick(int_t slotIndex, int_t mouseButton, bool shiftClick, EntityPlayer *entityplayer);

	virtual void onCraftGuiClosed(EntityPlayer *entityplayer);
	virtual void onCraftMatrixChanged(IInventory *iinventory);
	void putStackInSlot(int_t i, ItemStack *itemstack);
	void putStacksInSlots(ItemStack **aitemstack, int_t length);

	// Java: func_20112_a — pushes a progress-bar value to the open container.
	virtual void updateProgressBar(int_t id, int_t value);

	// Java: func_20111_a — unique transaction id for InventoryPlayer actions.
	short_t getNextTransactionID(InventoryPlayer *inventoryplayer);

	// Java: func_20113_a / func_20110_b — transaction confirm/response (server side).
	virtual void updateTransaction(short_t transactionID);
	virtual void onTransactionResponse(short_t transactionID);

	virtual bool isUsableByPlayer(EntityPlayer *entityplayer) = 0;

	// Not in Java. True when any of this container's slots is backed by
	// `iinventory`. World::closeContainersUsing() calls this just before a
	// TileEntity is deleted so the player's open screen is closed while the
	// pointer is still valid: Java could leave a GuiFurnace holding a removed
	// TileEntityFurnace indefinitely, here that same pointer is freed memory and
	// the next isUsableByPlayer() call goes through a dead vtable.
	bool usesInventory(IInventory *iinventory);

protected:
	virtual void retrySlotClick(int_t slotIndex, int_t mouseButton, bool shiftClick, EntityPlayer *entityplayer);

	// Java: func_28125_a — merge itemstack into slots in [startSlot, endSlot).
	bool mergeItemStack(ItemStack *itemstack, int_t startSlot, int_t endSlot, bool lastToFirst);

public:
	// Java field_20123_d — cached copies of slot contents for change detection.
	std::vector<ItemStack *> inventoryItemStacks;

	std::vector<Slot *> slots;
	int_t windowId = 0;

private:
	// Java field_20917_a — counter for generating unique transaction IDs.
	short_t transactionID = 0;

protected:
	// Java field_20121_g — list of ICrafting listeners (for GUI updates).
	std::vector<ICrafting *> crafters;

private:
	// Java field_20918_b — set of players who have this container open.
	std::set<int_t> playerList;
};
