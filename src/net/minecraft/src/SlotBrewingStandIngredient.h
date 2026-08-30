#pragma once

#include "Slot.h"

class ContainerBrewingStand;

// net.minecraft.src.SlotBrewingStandIngredient
class SlotBrewingStandIngredient : public Slot
{
public:
    SlotBrewingStandIngredient(ContainerBrewingStand *container, IInventory *inventory, int_t slot, int_t x, int_t y);

    bool isItemValid(ItemStack *itemstack) override;
    int_t getSlotStackLimit() override;

private:
    ContainerBrewingStand *container;
};
