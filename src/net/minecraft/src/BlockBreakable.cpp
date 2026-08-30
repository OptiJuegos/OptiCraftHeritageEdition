#include "BlockBreakable.h"
#include "IBlockAccess.h"

BlockBreakable::BlockBreakable(int_t i, int_t j, Material *material, bool flag)
    : Block(i, j, material), localFlag(flag)
{
}

bool BlockBreakable::isOpaqueCube()
{
    return false;
}

bool BlockBreakable::shouldSideBeRendered(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k, int_t l)
{
    int_t i1 = iblockaccess->getBlockId(i, j, k);
    if (!localFlag && i1 == blockID)
        return false;
    return Block::shouldSideBeRendered(iblockaccess, i, j, k, l);
}
