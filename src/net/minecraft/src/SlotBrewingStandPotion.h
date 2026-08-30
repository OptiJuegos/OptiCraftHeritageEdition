#pragma once

#include "Slot.h"

class ContainerBrewingStand;
class EntityPlayer;

// net.minecraft.src.SlotBrewingStandPotion
class SlotBrewingStandPotion : public Slot
{
public:
    SlotBrewingStandPotion(ContainerBrewingStand *container, EntityPlayer *player, IInventory *inventory,
                           int_t slot, int_t x, int_t y);

    bool isItemValid(ItemStack *itemstack) override;
    int_t getSlotStackLimit() override;
    void onPickupFromSlot(ItemStack *itemstack) override;

private:
    ContainerBrewingStand *container;
    EntityPlayer *player;
};
