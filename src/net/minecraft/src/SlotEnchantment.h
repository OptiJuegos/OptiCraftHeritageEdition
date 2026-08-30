#pragma once

#include "Slot.h"

class ContainerEnchantment;
class IInventory;
class ItemStack;

// net.minecraft.src.SlotEnchantment
class SlotEnchantment : public Slot
{
public:
	SlotEnchantment(ContainerEnchantment *container, IInventory *inventory, int_t slot, int_t x, int_t y);
	bool isItemValid(ItemStack *stack) override;

private:
	ContainerEnchantment *container;
};
