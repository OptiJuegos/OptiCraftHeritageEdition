#pragma once

#include "Item.h"
#include "EnumToolMaterial.h"

class ItemHoe : public Item {
public:
    ItemHoe(int id, EnumToolMaterial material);

    bool onItemUse(ItemStack* itemstack, EntityPlayer* entityplayer, World* world, int i, int j, int k, int l) override;
    bool isFull3D() override;
};
