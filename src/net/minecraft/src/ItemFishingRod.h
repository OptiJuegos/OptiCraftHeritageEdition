#pragma once

#include "Item.h"

class ItemFishingRod : public Item {
public:
    ItemFishingRod(int id);

    bool isFull3D() override;
    bool shouldRotateAroundWhenRendering() override;
    ItemStack* onItemRightClick(ItemStack* itemstack, World* world, EntityPlayer* entityplayer) override;
};
