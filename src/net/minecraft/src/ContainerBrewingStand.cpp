#include "ContainerBrewingStand.h"

#include "EntityPlayer.h"
#include "ICrafting.h"
#include "InventoryPlayer.h"
#include "ItemStack.h"
#include "Slot.h"
#include "SlotBrewingStandIngredient.h"
#include "SlotBrewingStandPotion.h"
#include "TileEntityBrewingStand.h"

ContainerBrewingStand::ContainerBrewingStand(InventoryPlayer *inventory, TileEntityBrewingStand *brewingStand)
    : tileBrewingStand(brewingStand)
{
    ownsBrewingStand = brewingStand != nullptr && brewingStand->worldObj == nullptr;
    addSlot(new SlotBrewingStandPotion(this, inventory->player, brewingStand, 0, 56, 46));
    addSlot(new SlotBrewingStandPotion(this, inventory->player, brewingStand, 1, 79, 53));
    addSlot(new SlotBrewingStandPotion(this, inventory->player, brewingStand, 2, 102, 46));
    addSlot(new SlotBrewingStandIngredient(this, brewingStand, 3, 79, 17));

    for (int row = 0; row < 3; ++row)
    {
        for (int column = 0; column < 9; ++column)
            addSlot(new Slot(inventory, column + row * 9 + 9, 8 + column * 18, 84 + row * 18));
    }
    for (int column = 0; column < 9; ++column)
        addSlot(new Slot(inventory, column, 8 + column * 18, 142));
}

ContainerBrewingStand::~ContainerBrewingStand()
{
    if (ownsBrewingStand)
        delete tileBrewingStand;
}

void ContainerBrewingStand::updateCraftingResults()
{
    Container::updateCraftingResults();
    for (ICrafting *crafter : crafters)
    {
        if (crafter != nullptr && brewTime != tileBrewingStand->getBrewTime())
            crafter->sendProgressBarUpdate(this, 0, tileBrewingStand->getBrewTime());
    }
    brewTime = tileBrewingStand->getBrewTime();
}

void ContainerBrewingStand::updateProgressBar(int_t id, int_t value)
{
    if (id == 0)
        tileBrewingStand->setBrewTime(value);
}

bool ContainerBrewingStand::isUsableByPlayer(EntityPlayer *player)
{
    return tileBrewingStand != nullptr && tileBrewingStand->canInteractWith(player);
}

ItemStack *ContainerBrewingStand::getStackInSlot(int_t slotIndex)
{
    if (slotIndex < 0 || slotIndex >= static_cast<int_t>(slots.size()))
        return nullptr;

    Slot *slot = slots[slotIndex];
    if (slot == nullptr || !slot->getHasStack())
        return nullptr;

    ItemStack *source = slot->getStack();
    ItemStack *original = source->copy();
    bool moved = false;

    if ((slotIndex < 0 || slotIndex > 2) && slotIndex != 3)
    {
        if (slotIndex >= 4 && slotIndex < 31)
            moved = mergeItemStack(source, 31, 40, false);
        else if (slotIndex >= 31 && slotIndex < 40)
            moved = mergeItemStack(source, 4, 31, false);
        else
            moved = mergeItemStack(source, 4, 40, false);
    }
    else
    {
        moved = mergeItemStack(source, 4, 40, true);
        if (moved)
            slot->onSlotChange(source, original);
    }

    if (!moved)
    {
        delete original;
        return nullptr;
    }

    ItemStack *orphaned = nullptr;
    if (source->stackSize == 0)
    {
        orphaned = slot->takeStack();
    }
    else
    {
        slot->onSlotChanged();
    }

    if (source->stackSize == original->stackSize)
    {
        delete orphaned;
        delete original;
        return nullptr;
    }

    slot->onPickupFromSlot(source);
    delete orphaned;
    return original;
}
