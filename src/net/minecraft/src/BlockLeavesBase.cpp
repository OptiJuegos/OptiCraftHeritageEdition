#include "BlockLeavesBase.h"
#include "IBlockAccess.h"

BlockLeavesBase::BlockLeavesBase(int_t i, int_t j, Material *material, bool flag)
    : Block(i, j, material), graphicsLevel(flag)
{
}

bool BlockLeavesBase::isOpaqueCube()
{
    return false;
}

bool BlockLeavesBase::shouldSideBeRendered(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k, int_t l)
{
    int_t i1 = iblockaccess->getBlockId(i, j, k);
    if (!graphicsLevel && i1 == blockID)
        return false;
    return Block::shouldSideBeRendered(iblockaccess, i, j, k, l);
}
