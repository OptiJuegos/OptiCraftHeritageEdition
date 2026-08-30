#include "ItemCoal.h"
#include "ItemStack.h"

ItemCoal::ItemCoal(int i)
    : Item(i) {
    setHasSubtypes(true);
    setMaxDamage(0);
}

std::string ItemCoal::getItemNameIS(ItemStack* itemstack) {
    if (itemstack->getItemDamage() == 1) {
        return "item.charcoal";
    } else {
        return "item.coal";
    }
}
