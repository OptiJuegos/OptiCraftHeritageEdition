#include "SlotBrewingStandPotion.h"

#include "AchievementList.h"
#include "ContainerBrewingStand.h"
#include "EntityPlayer.h"
#include "Item.h"
#include "ItemStack.h"

SlotBrewingStandPotion::SlotBrewingStandPotion(ContainerBrewingStand *containerIn, EntityPlayer *playerIn,
                                               IInventory *inventory, int_t slot, int_t x, int_t y)
    : Slot(inventory, slot, x, y), container(containerIn), player(playerIn)
{
}

bool SlotBrewingStandPotion::isItemValid(ItemStack *itemstack)
{
    return itemstack != nullptr && Item::potion != nullptr && Item::glassBottle != nullptr &&
           (itemstack->itemID == Item::potion->shiftedIndex || itemstack->itemID == Item::glassBottle->shiftedIndex);
}

int_t SlotBrewingStandPotion::getSlotStackLimit()
{
    return 1;
}

void SlotBrewingStandPotion::onPickupFromSlot(ItemStack *itemstack)
{
    if (itemstack != nullptr && player != nullptr && Item::potion != nullptr &&
        itemstack->itemID == Item::potion->shiftedIndex && itemstack->getItemDamage() > 0)
    {
        player->addStat(AchievementList::potion, 1);
    }
    Slot::onPickupFromSlot(itemstack);
}
