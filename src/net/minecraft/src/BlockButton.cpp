#include "BlockButton.h"
#include "Material.h"
#include "World.h"
#include "IBlockAccess.h"
#include "AxisAlignedBB.h"
#include "EntityPlayer.h"
#include "Block.h"

BlockButton::BlockButton(int_t i, int_t j) : Block(i, j, Material::circuits)
{
    setTickOnLoad(true);
}

AxisAlignedBB *BlockButton::getCollisionBoundingBoxFromPool(World *world, int_t i, int_t j, int_t k)
{
    return nullptr;
}

int_t BlockButton::tickRate()         { return 20; }
bool  BlockButton::isOpaqueCube()     { return false; }
bool  BlockButton::renderAsNormalBlock() { return false; }

bool BlockButton::canPlaceBlockOnSide(World *world, int_t i, int_t j, int_t k, int_t l)
{
    if (l == 2 && world->isBlockNormalCube(i, j, k + 1)) return true;
    if (l == 3 && world->isBlockNormalCube(i, j, k - 1)) return true;
    if (l == 4 && world->isBlockNormalCube(i + 1, j, k)) return true;
    return l == 5 && world->isBlockNormalCube(i - 1, j, k);
}

bool BlockButton::canPlaceBlockAt(World *world, int_t i, int_t j, int_t k)
{
    if (world->isBlockNormalCube(i - 1, j, k)) return true;
    if (world->isBlockNormalCube(i + 1, j, k)) return true;
    if (world->isBlockNormalCube(i, j, k - 1)) return true;
    return world->isBlockNormalCube(i, j, k + 1);
}

void BlockButton::onBlockPlaced(World *world, int_t i, int_t j, int_t k, int_t l)
{
    int_t i1 = world->getBlockMetadata(i, j, k);
    int_t j1 = i1 & 8;
    i1 &= 7;
    if      (l == 2 && world->isBlockNormalCube(i, j, k + 1)) i1 = 4;
    else if (l == 3 && world->isBlockNormalCube(i, j, k - 1)) i1 = 3;
    else if (l == 4 && world->isBlockNormalCube(i + 1, j, k)) i1 = 2;
    else if (l == 5 && world->isBlockNormalCube(i - 1, j, k)) i1 = 1;
    else                                                        i1 = getOrientation(world, i, j, k);
    world->setBlockMetadataWithNotify(i, j, k, i1 + j1);
}

int_t BlockButton::getOrientation(World *world, int_t i, int_t j, int_t k)
{
    if (world->isBlockNormalCube(i - 1, j, k)) return 1;
    if (world->isBlockNormalCube(i + 1, j, k)) return 2;
    if (world->isBlockNormalCube(i, j, k - 1)) return 3;
    return !world->isBlockNormalCube(i, j, k + 1) ? 1 : 4;
}

void BlockButton::onNeighborBlockChange(World *world, int_t i, int_t j, int_t k, int_t l)
{
    if (getLiquidBlockID(world, i, j, k))
    {
        int_t i1  = world->getBlockMetadata(i, j, k) & 7;
        bool flag = false;
        if (!world->isBlockNormalCube(i - 1, j, k) && i1 == 1) flag = true;
        if (!world->isBlockNormalCube(i + 1, j, k) && i1 == 2) flag = true;
        if (!world->isBlockNormalCube(i, j, k - 1) && i1 == 3) flag = true;
        if (!world->isBlockNormalCube(i, j, k + 1) && i1 == 4) flag = true;
        if (flag)
        {
            dropBlockAsItem(world, i, j, k, world->getBlockMetadata(i, j, k));
            world->setBlockWithNotify(i, j, k, 0);
        }
    }
}

bool BlockButton::getLiquidBlockID(World *world, int_t i, int_t j, int_t k)
{
    if (!canPlaceBlockAt(world, i, j, k))
    {
        dropBlockAsItem(world, i, j, k, world->getBlockMetadata(i, j, k));
        world->setBlockWithNotify(i, j, k, 0);
        return false;
    }
    return true;
}

void BlockButton::setBlockBoundsBasedOnState(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k)
{
    int_t l   = iblockaccess->getBlockMetadata(i, j, k);
    int_t i1  = l & 7;
    bool flag = (l & 8) > 0;
    float f   = 0.375f;
    float f1  = 0.625f;
    float f2  = 0.1875f;
    float f3  = flag ? 0.0625f : 0.125f;
    if (i1 == 1) setBlockBounds(0.0f,       f, 0.5f - f2, f3,       f1, 0.5f + f2);
    else if (i1 == 2) setBlockBounds(1.0f - f3, f, 0.5f - f2, 1.0f,     f1, 0.5f + f2);
    else if (i1 == 3) setBlockBounds(0.5f - f2, f, 0.0f,      0.5f + f2, f1, f3);
    else if (i1 == 4) setBlockBounds(0.5f - f2, f, 1.0f - f3, 0.5f + f2, f1, 1.0f);
}

void BlockButton::onBlockClicked(World *world, int_t i, int_t j, int_t k, EntityPlayer *entityplayer)
{
    blockActivated(world, i, j, k, entityplayer);
}

bool BlockButton::blockActivated(World *world, int_t i, int_t j, int_t k, EntityPlayer *entityplayer)
{
    int_t l  = world->getBlockMetadata(i, j, k);
    int_t i1 = l & 7;
    int_t j1 = 8 - (l & 8);
    if (j1 == 0) return true;
    world->setBlockMetadataWithNotify(i, j, k, i1 + j1);
    world->markBlocksDirty(i, j, k, i, j, k);
    world->playSoundEffect((double)i + 0.5, (double)j + 0.5, (double)k + 0.5, "random.click", 0.3f, 0.6f);
    world->notifyBlocksOfNeighborChange(i, j, k, blockID);
    if (i1 == 1)      world->notifyBlocksOfNeighborChange(i - 1, j, k, blockID);
    else if (i1 == 2) world->notifyBlocksOfNeighborChange(i + 1, j, k, blockID);
    else if (i1 == 3) world->notifyBlocksOfNeighborChange(i, j, k - 1, blockID);
    else if (i1 == 4) world->notifyBlocksOfNeighborChange(i, j, k + 1, blockID);
    else              world->notifyBlocksOfNeighborChange(i, j - 1, k, blockID);
    world->scheduleBlockUpdate(i, j, k, blockID, tickRate());
    return true;
}

void BlockButton::onBlockRemoval(World *world, int_t i, int_t j, int_t k)
{
    int_t l = world->getBlockMetadata(i, j, k);
    if ((l & 8) > 0)
    {
        world->notifyBlocksOfNeighborChange(i, j, k, blockID);
        int_t i1 = l & 7;
        if (i1 == 1)      world->notifyBlocksOfNeighborChange(i - 1, j, k, blockID);
        else if (i1 == 2) world->notifyBlocksOfNeighborChange(i + 1, j, k, blockID);
        else if (i1 == 3) world->notifyBlocksOfNeighborChange(i, j, k - 1, blockID);
        else if (i1 == 4) world->notifyBlocksOfNeighborChange(i, j, k + 1, blockID);
        else              world->notifyBlocksOfNeighborChange(i, j - 1, k, blockID);
    }
    Block::onBlockRemoval(world, i, j, k);
}

bool BlockButton::isPoweringTo(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k, int_t l)
{
    return (iblockaccess->getBlockMetadata(i, j, k) & 8) > 0;
}

bool BlockButton::isIndirectlyPoweringTo(World *world, int_t i, int_t j, int_t k, int_t l)
{
    int_t i1 = world->getBlockMetadata(i, j, k);
    if ((i1 & 8) == 0) return false;
    int_t j1 = i1 & 7;
    if (j1 == 5 && l == 1) return true;
    if (j1 == 4 && l == 2) return true;
    if (j1 == 3 && l == 3) return true;
    if (j1 == 2 && l == 4) return true;
    return j1 == 1 && l == 5;
}

bool BlockButton::canProvidePower() { return true; }

void BlockButton::updateTick(World *world, int_t i, int_t j, int_t k, Random &random)
{
    if (world->multiplayerWorld) return;
    int_t l = world->getBlockMetadata(i, j, k);
    if ((l & 8) == 0) return;
    world->setBlockMetadataWithNotify(i, j, k, l & 7);
    world->notifyBlocksOfNeighborChange(i, j, k, blockID);
    int_t i1 = l & 7;
    if (i1 == 1)      world->notifyBlocksOfNeighborChange(i - 1, j, k, blockID);
    else if (i1 == 2) world->notifyBlocksOfNeighborChange(i + 1, j, k, blockID);
    else if (i1 == 3) world->notifyBlocksOfNeighborChange(i, j, k - 1, blockID);
    else if (i1 == 4) world->notifyBlocksOfNeighborChange(i, j, k + 1, blockID);
    else              world->notifyBlocksOfNeighborChange(i, j - 1, k, blockID);
    world->playSoundEffect((double)i + 0.5, (double)j + 0.5, (double)k + 0.5, "random.click", 0.3f, 0.5f);
    world->markBlocksDirty(i, j, k, i, j, k);
}

void BlockButton::setBlockBoundsForItemRender()
{
    float f  = 0.1875f;
    float f1 = 0.125f;
    float f2 = 0.125f;
    setBlockBounds(0.5f - f, 0.5f - f1, 0.5f - f2, 0.5f + f, 0.5f + f1, 0.5f + f2);
}
