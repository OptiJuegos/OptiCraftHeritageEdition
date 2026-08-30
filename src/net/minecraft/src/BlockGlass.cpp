#include "BlockGlass.h"

BlockGlass::BlockGlass(int_t i, int_t j, Material *material, bool flag)
    : BlockBreakable(i, j, material, flag)
{
}

int_t BlockGlass::quantityDropped(Random &random)
{
    return 0;
}

int_t BlockGlass::getRenderBlockPass()
{
    return 0;
}

bool BlockGlass::isOpaqueCube()
{
    return false;
}

bool BlockGlass::renderAsNormalBlock()
{
    return false;
}

bool BlockGlass::func_50074_q()
{
    return true;
}
