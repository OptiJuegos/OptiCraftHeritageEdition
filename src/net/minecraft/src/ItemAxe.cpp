#include "ItemAxe.h"
#include "Block.h"
#include "Material.h"

Block* ItemAxe::blocksEffectiveAgainst[8] = {};
int ItemAxe::numBlocksEffectiveAgainst = 8;

Block** ItemAxe::getBlocksEffectiveAgainst() {
    Block* blocks[] = {
        Block::planks, Block::bookShelf, Block::wood, Block::chest,
        Block::stairDouble, Block::stairSingle, Block::pumpkin, Block::pumpkinLantern
    };
    for (int i = 0; i < numBlocksEffectiveAgainst; i++) {
        blocksEffectiveAgainst[i] = blocks[i];
    }
    return blocksEffectiveAgainst;
}

ItemAxe::ItemAxe(int i, EnumToolMaterial enumtoolmaterial)
    : ItemTool(i, 3, enumtoolmaterial, getBlocksEffectiveAgainst(), numBlocksEffectiveAgainst) {
}

float ItemAxe::getStrVsBlock(ItemStack* itemstack, Block* block) {
    if (block != nullptr && block->blockMaterial == Material::wood) {
        return efficiencyOnProperMaterial;
    }
    return ItemTool::getStrVsBlock(itemstack, block);
}
