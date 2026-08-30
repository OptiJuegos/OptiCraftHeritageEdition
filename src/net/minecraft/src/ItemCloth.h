#pragma once

#include "ItemBlock.h"

class ItemCloth : public ItemBlock {
public:
    ItemCloth(int id);

    int getIconFromDamage(int damage) override;
    int getMetadata(int damage) override;
    std::string getItemNameIS(ItemStack* itemstack) override;
};
