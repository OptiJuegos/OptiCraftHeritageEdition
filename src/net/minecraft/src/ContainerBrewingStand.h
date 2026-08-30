#pragma once

#include "Container.h"

class InventoryPlayer;
class TileEntityBrewingStand;
class EntityPlayer;
class ItemStack;

// net.minecraft.src.ContainerBrewingStand
class ContainerBrewingStand : public Container
{
public:
    ContainerBrewingStand(InventoryPlayer *inventory, TileEntityBrewingStand *brewingStand);
    ~ContainerBrewingStand() override;

    void updateCraftingResults() override;
    void updateProgressBar(int_t id, int_t value) override;
    bool isUsableByPlayer(EntityPlayer *player) override;
    ItemStack *getStackInSlot(int_t slot) override;

private:
    TileEntityBrewingStand *tileBrewingStand;
    bool ownsBrewingStand = false;
    int_t brewTime = 0;
};
