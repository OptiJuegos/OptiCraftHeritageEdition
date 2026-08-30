#pragma once

#include "ItemTool.h"

class ItemSpade : public ItemTool {
public:
    ItemSpade(int id, EnumToolMaterial material);

    bool canHarvestBlock(Block* block) override;

private:
    static Block* blocksEffectiveAgainst[];
    static int numBlocksEffectiveAgainst;
    static Block** getBlocksEffectiveAgainst();
};
