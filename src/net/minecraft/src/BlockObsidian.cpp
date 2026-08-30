#include "BlockObsidian.h"
#include "Block.h"

BlockObsidian::BlockObsidian(int_t i, int_t j) : BlockStone(i, j)
{
}

int_t BlockObsidian::quantityDropped(Random &random)
{
    return 1;
}

int_t BlockObsidian::idDropped(int_t i, Random &random)
{
    return Block::obsidian->blockID;
}
