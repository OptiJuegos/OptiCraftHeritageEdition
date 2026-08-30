#include "ItemSpade.h"
#include "Block.h"
#include "BlockGrass.h"

Block* ItemSpade::blocksEffectiveAgainst[8] = {};
int ItemSpade::numBlocksEffectiveAgainst = 8;

Block** ItemSpade::getBlocksEffectiveAgainst() {
    Block* blocks[] = {
        Block::grass, Block::dirt, Block::sand, Block::gravel,
        Block::snow, Block::blockSnow, Block::blockClay, Block::tilledField
    };
    for (int i = 0; i < numBlocksEffectiveAgainst; i++) {
        blocksEffectiveAgainst[i] = blocks[i];
    }
    return blocksEffectiveAgainst;
}

ItemSpade::ItemSpade(int i, EnumToolMaterial enumtoolmaterial)
    : ItemTool(i, 1, enumtoolmaterial, getBlocksEffectiveAgainst(), numBlocksEffectiveAgainst) {
}

bool ItemSpade::canHarvestBlock(Block* block) {
    if (block == Block::snow) return true;
    return block == Block::blockSnow;
}
