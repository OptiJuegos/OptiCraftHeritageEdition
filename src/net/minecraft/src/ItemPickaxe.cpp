#include "ItemPickaxe.h"
#include "Block.h"
#include "Material.h"

Block* ItemPickaxe::blocksEffectiveAgainst[23] = {};
int ItemPickaxe::numBlocksEffectiveAgainst = 23;

Block** ItemPickaxe::getBlocksEffectiveAgainst() {
    Block* blocks[] = {
        Block::cobblestone, Block::stairDouble, Block::stairSingle, Block::stone,
        Block::sandStone, Block::cobblestoneMossy, Block::oreIron, Block::blockSteel,
        Block::oreCoal, Block::blockGold, Block::oreGold, Block::oreDiamond,
        Block::blockDiamond, Block::ice, Block::netherrack, Block::oreLapis,
        Block::blockLapis, Block::oreRedstone, Block::oreRedstoneGlowing, Block::rail,
        Block::railDetector, Block::railPowered
    };
    for (int i = 0; i < numBlocksEffectiveAgainst; i++) {
        blocksEffectiveAgainst[i] = blocks[i];
    }
    return blocksEffectiveAgainst;
}

ItemPickaxe::ItemPickaxe(int i, EnumToolMaterial enumtoolmaterial)
    : ItemTool(i, 2, enumtoolmaterial, getBlocksEffectiveAgainst(), numBlocksEffectiveAgainst) {
}

bool ItemPickaxe::canHarvestBlock(Block* block) {
    if (block == Block::obsidian) {
        return EnumToolMaterialHelper::getHarvestLevel(toolMaterial) == 3;
    }
    if (block == Block::blockDiamond || block == Block::oreDiamond) {
        return EnumToolMaterialHelper::getHarvestLevel(toolMaterial) >= 2;
    }
    if (block == Block::blockGold || block == Block::oreGold) {
        return EnumToolMaterialHelper::getHarvestLevel(toolMaterial) >= 2;
    }
    if (block == Block::blockSteel || block == Block::oreIron) {
        return EnumToolMaterialHelper::getHarvestLevel(toolMaterial) >= 1;
    }
    if (block == Block::blockLapis || block == Block::oreLapis) {
        return EnumToolMaterialHelper::getHarvestLevel(toolMaterial) >= 1;
    }
    if (block == Block::oreRedstone || block == Block::oreRedstoneGlowing) {
        return EnumToolMaterialHelper::getHarvestLevel(toolMaterial) >= 2;
    }
    if (block->blockMaterial == Material::rock) {
        return true;
    }
    return block->blockMaterial == Material::iron;
}

float ItemPickaxe::getStrVsBlock(ItemStack* itemstack, Block* block) {
    if (block != nullptr && (block->blockMaterial == Material::iron || block->blockMaterial == Material::rock)) {
        return efficiencyOnProperMaterial;
    }
    return ItemTool::getStrVsBlock(itemstack, block);
}
