#pragma once

#include "ItemTool.h"

class ItemPickaxe : public ItemTool {
public:
    ItemPickaxe(int id, EnumToolMaterial material);

    bool canHarvestBlock(Block* block) override;
    float getStrVsBlock(ItemStack* itemstack, Block* block) override;

private:
    static Block* blocksEffectiveAgainst[];
    static int numBlocksEffectiveAgainst;
    static Block** getBlocksEffectiveAgainst();
};
