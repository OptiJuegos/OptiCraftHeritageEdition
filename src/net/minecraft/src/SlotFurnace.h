#pragma once

#include "Slot.h"

class EntityPlayer;
class IInventory;
class ItemStack;

// net.minecraft.src.SlotFurnace
class SlotFurnace : public Slot
{
public:
	SlotFurnace(EntityPlayer *entityplayer, IInventory *iinventory, int_t i, int_t j, int_t k);

	bool isItemValid(ItemStack *itemstack) override;
	ItemStack *decrStackSize(int_t amount) override;
	void onPickupFromSlot(ItemStack *itemstack) override;

protected:
	void onCrafting(ItemStack *itemstack, int_t amount) override;
	void onCrafting(ItemStack *itemstack) override;

private:
	EntityPlayer *thePlayer;
	int_t removeCount = 0;
};
