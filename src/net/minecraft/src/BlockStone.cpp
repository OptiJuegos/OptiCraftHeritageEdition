#include "BlockStone.h"
#include "Material.h"
#include "Block.h"

BlockStone::BlockStone(int_t i, int_t j) : Block(i, j, Material::rock)
{
}

int_t BlockStone::idDropped(int_t i, Random &random)
{
    return Block::cobblestone->blockID;
}
