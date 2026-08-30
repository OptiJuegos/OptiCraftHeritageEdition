#include "SlotBrewingStandIngredient.h"

#include "ContainerBrewingStand.h"
#include "Item.h"
#include "ItemStack.h"

SlotBrewingStandIngredient::SlotBrewingStandIngredient(ContainerBrewingStand *containerIn, IInventory *inventory,
                                                       int_t slot, int_t x, int_t y)
    : Slot(inventory, slot, x, y), container(containerIn)
{
}

bool SlotBrewingStandIngredient::isItemValid(ItemStack *itemstack)
{
    (void)container;
    if (itemstack == nullptr || !ItemStack::isValidItemID(itemstack->itemID))
        return false;
    Item *item = Item::itemsList[itemstack->itemID];
    return item != nullptr && item->isPotionIngredient();
}

int_t SlotBrewingStandIngredient::getSlotStackLimit()
{
    return 64;
}
