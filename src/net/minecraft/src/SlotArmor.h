#pragma once

#include "Slot.h"

class ContainerPlayer;
class IInventory;
class ItemStack;

// net.minecraft.src.SlotArmor
class SlotArmor : public Slot
{
public:
	SlotArmor(ContainerPlayer *containerplayer, IInventory *iinventory, int_t i, int_t j, int_t k, int_t l);

	int_t getSlotStackLimit() override;
	bool isItemValid(ItemStack *itemstack) override;
	int_t getBackgroundIconIndex() override;

	const int_t armorType;
	ContainerPlayer * const inventory;
};
