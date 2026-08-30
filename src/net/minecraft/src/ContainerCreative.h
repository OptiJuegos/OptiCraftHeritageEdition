#pragma once

#include "Container.h"
#include <vector>

class EntityPlayer;
class ItemStack;

// net.minecraft.src.ContainerCreative
class ContainerCreative : public Container
{
public:
    explicit ContainerCreative(EntityPlayer *player);
    ~ContainerCreative() override;

    bool isUsableByPlayer(EntityPlayer *player) override;
    void scrollTo(float_t scroll);

protected:
    void retrySlotClick(int_t slot, int_t button, bool shift, EntityPlayer *player) override;

public:
    std::vector<ItemStack *> itemList;
};
