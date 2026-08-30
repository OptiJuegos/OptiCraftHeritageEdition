#pragma once

#include "Item.h"

class ItemSaddle : public Item {
public:
    ItemSaddle(int id);

    void useItemOnEntity(ItemStack* itemstack, EntityLiving* entityliving) override;
    void saddleEntity(ItemStack* itemstack, EntityLiving* entityliving) override;
    bool hitEntity(ItemStack* itemstack, EntityLiving* entityliving, EntityLiving* entityliving1) override;
};
