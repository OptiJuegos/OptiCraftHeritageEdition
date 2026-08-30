#pragma once

#include "ItemTool.h"

class ItemAxe : public ItemTool {
public:
    ItemAxe(int id, EnumToolMaterial material);

    float getStrVsBlock(ItemStack* itemstack, Block* block) override;

private:
    static Block* blocksEffectiveAgainst[];
    static int numBlocksEffectiveAgainst;
    static Block** getBlocksEffectiveAgainst();
};
