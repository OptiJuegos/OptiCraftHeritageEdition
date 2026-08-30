#include "BlockLever.h"
#include "Material.h"
#include "World.h"
#include "IBlockAccess.h"
#include "AxisAlignedBB.h"
#include "EntityPlayer.h"
#include "Block.h"

BlockLever::BlockLever(int_t i, int_t j) : Block(i, j, Material::circuits)
{
}

AxisAlignedBB *BlockLever::getCollisionBoundingBoxFromPool(World *world, int_t i, int_t j, int_t k)
{
    return nullptr;
}

bool  BlockLever::isOpaqueCube()        { return false; }
bool  BlockLever::renderAsNormalBlock() { return false; }
int_t BlockLever::getRenderType()       { return 12; }

bool BlockLever::canPlaceBlockOnSide(World *world, int_t i, int_t j, int_t k, int_t l)
{
    if (l == 1 && world->isBlockNormalCube(i, j - 1, k)) return true;
    if (l == 2 && world->isBlockNormalCube(i, j, k + 1)) return true;
    if (l == 3 && world->isBlockNormalCube(i, j, k - 1)) return true;
    if (l == 4 && world->isBlockNormalCube(i + 1, j, k)) return true;
    return l == 5 && world->isBlockNormalCube(i - 1, j, k);
}

bool BlockLever::canPlaceBlockAt(World *world, int_t i, int_t j, int_t k)
{
    if (world->isBlockNormalCube(i - 1, j, k)) return true;
    if (world->isBlockNormalCube(i + 1, j, k)) return true;
    if (world->isBlockNormalCube(i, j, k - 1)) return true;
    if (world->isBlockNormalCube(i, j, k + 1)) return true;
    return world->isBlockNormalCube(i, j - 1, k);
}

void BlockLever::onBlockPlaced(World *world, int_t i, int_t j, int_t k, int_t l)
{
    int_t i1 = world->getBlockMetadata(i, j, k);
    int_t j1 = i1 & 8;
    i1 &= 7;
    i1 = -1;
    if (l == 1 && world->isBlockNormalCube(i, j - 1, k)) i1 = 5 + world->rand.nextInt(2);
    if (l == 2 && world->isBlockNormalCube(i, j, k + 1)) i1 = 4;
    if (l == 3 && world->isBlockNormalCube(i, j, k - 1)) i1 = 3;
    if (l == 4 && world->isBlockNormalCube(i + 1, j, k)) i1 = 2;
    if (l == 5 && world->isBlockNormalCube(i - 1, j, k)) i1 = 1;
    if (i1 == -1)
    {
        dropBlockAsItem(world, i, j, k, world->getBlockMetadata(i, j, k));
        world->setBlockWithNotify(i, j, k, 0);
    }
    else
    {
        world->setBlockMetadataWithNotify(i, j, k, i1 + j1);
    }
}

void BlockLever::onNeighborBlockChange(World *world, int_t i, int_t j, int_t k, int_t l)
{
    if (checkIfAttachedToBlock(world, i, j, k))
    {
        int_t i1  = world->getBlockMetadata(i, j, k) & 7;
        bool flag = false;
        if (!world->isBlockNormalCube(i - 1, j, k) && i1 == 1) flag = true;
        if (!world->isBlockNormalCube(i + 1, j, k) && i1 == 2) flag = true;
        if (!world->isBlockNormalCube(i, j, k - 1) && i1 == 3) flag = true;
        if (!world->isBlockNormalCube(i, j, k + 1) && i1 == 4) flag = true;
        if (!world->isBlockNormalCube(i, j - 1, k) && i1 == 5) flag = true;
        if (!world->isBlockNormalCube(i, j - 1, k) && i1 == 6) flag = true;
        if (flag)
        {
            dropBlockAsItem(world, i, j, k, world->getBlockMetadata(i, j, k));
            world->setBlockWithNotify(i, j, k, 0);
        }
    }
}

bool BlockLever::checkIfAttachedToBlock(World *world, int_t i, int_t j, int_t k)
{
    if (!canPlaceBlockAt(world, i, j, k))
    {
        dropBlockAsItem(world, i, j, k, world->getBlockMetadata(i, j, k));
        world->setBlockWithNotify(i, j, k, 0);
        return false;
    }
    return true;
}

void BlockLever::setBlockBoundsBasedOnState(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k)
{
    int_t l = iblockaccess->getBlockMetadata(i, j, k) & 7;
    float f = 0.1875f;
    if (l == 1)      setBlockBounds(0.0f,       0.2f, 0.5f - f, f * 2.0f, 0.8f, 0.5f + f);
    else if (l == 2) setBlockBounds(1.0f - f * 2.0f, 0.2f, 0.5f - f, 1.0f, 0.8f, 0.5f + f);
    else if (l == 3) setBlockBounds(0.5f - f, 0.2f, 0.0f, 0.5f + f, 0.8f, f * 2.0f);
    else if (l == 4) setBlockBounds(0.5f - f, 0.2f, 1.0f - f * 2.0f, 0.5f + f, 0.8f, 1.0f);
    else
    {
        float f1 = 0.25f;
        setBlockBounds(0.5f - f1, 0.0f, 0.5f - f1, 0.5f + f1, 0.6f, 0.5f + f1);
    }
}

void BlockLever::onBlockClicked(World *world, int_t i, int_t j, int_t k, EntityPlayer *entityplayer)
{
    blockActivated(world, i, j, k, entityplayer);
}

bool BlockLever::blockActivated(World *world, int_t i, int_t j, int_t k, EntityPlayer *entityplayer)
{
    if (world->multiplayerWorld) return true;
    int_t l  = world->getBlockMetadata(i, j, k);
    int_t i1 = l & 7;
    int_t j1 = 8 - (l & 8);
    world->setBlockMetadataWithNotify(i, j, k, i1 + j1);
    world->markBlocksDirty(i, j, k, i, j, k);
    world->playSoundEffect((double)i + 0.5, (double)j + 0.5, (double)k + 0.5,
                           "random.click", 0.3f, j1 <= 0 ? 0.5f : 0.6f);
    world->notifyBlocksOfNeighborChange(i, j, k, blockID);
    if (i1 == 1)      world->notifyBlocksOfNeighborChange(i - 1, j, k, blockID);
    else if (i1 == 2) world->notifyBlocksOfNeighborChange(i + 1, j, k, blockID);
    else if (i1 == 3) world->notifyBlocksOfNeighborChange(i, j, k - 1, blockID);
    else if (i1 == 4) world->notifyBlocksOfNeighborChange(i, j, k + 1, blockID);
    else              world->notifyBlocksOfNeighborChange(i, j - 1, k, blockID);
    return true;
}

void BlockLever::onBlockRemoval(World *world, int_t i, int_t j, int_t k)
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

bool BlockLever::isPoweringTo(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k, int_t l)
{
    return (iblockaccess->getBlockMetadata(i, j, k) & 8) > 0;
}

bool BlockLever::isIndirectlyPoweringTo(World *world, int_t i, int_t j, int_t k, int_t l)
{
    int_t i1 = world->getBlockMetadata(i, j, k);
    if ((i1 & 8) == 0) return false;
    int_t j1 = i1 & 7;
    if (j1 == 6 && l == 1) return true;
    if (j1 == 5 && l == 1) return true;
    if (j1 == 4 && l == 2) return true;
    if (j1 == 3 && l == 3) return true;
    if (j1 == 2 && l == 4) return true;
    return j1 == 1 && l == 5;
}

bool BlockLever::canProvidePower() { return true; }
