#include "BlockRedstoneRepeater.h"
#include "Material.h"
#include "World.h"
#include "IBlockAccess.h"
#include "Block.h"
#include "Item.h"
#include "EntityLiving.h"
#include "EntityPlayer.h"
#include "MathHelper.h"

const double BlockRedstoneRepeater::isPowered[4] = {-0.0625, 0.0625, 0.1875, 0.3125};
const int_t  BlockRedstoneRepeater::tickRate[4] = {1, 2, 3, 4};

BlockRedstoneRepeater::BlockRedstoneRepeater(int_t i, bool flag)
    : Block(i, 6, Material::circuits)
{
    isRepeaterPowered = flag;
    setBlockBounds(0.0f, 0.0f, 0.0f, 1.0f, 0.125f, 1.0f);
}

bool BlockRedstoneRepeater::renderAsNormalBlock() { return false; }

bool BlockRedstoneRepeater::canPlaceBlockAt(World *world, int_t i, int_t j, int_t k)
{
    if (!world->isBlockNormalCube(i, j - 1, k)) return false;
    return Block::canPlaceBlockAt(world, i, j, k);
}

bool BlockRedstoneRepeater::canBlockStay(World *world, int_t i, int_t j, int_t k)
{
    if (!world->isBlockNormalCube(i, j - 1, k)) return false;
    return Block::canBlockStay(world, i, j, k);
}

void BlockRedstoneRepeater::updateTick(World *world, int_t i, int_t j, int_t k, Random &random)
{
    int_t l    = world->getBlockMetadata(i, j, k);
    bool flag  = getBlockTextureFromSideAndMetadata(world, i, j, k, l);
    if (isRepeaterPowered && !flag)
    {
        world->setBlockAndMetadataWithNotify(i, j, k, Block::redstoneRepeaterIdle->blockID, l);
    }
    else if (!isRepeaterPowered)
    {
        world->setBlockAndMetadataWithNotify(i, j, k, Block::redstoneRepeaterActive->blockID, l);
        if (!flag)
        {
            int_t i1 = (l & 0xc) >> 2;
            world->scheduleBlockUpdate(i, j, k, Block::redstoneRepeaterActive->blockID, tickRate[i1] * 2);
        }
    }
}

int_t BlockRedstoneRepeater::getBlockTextureFromSideAndMetadata(int_t i, int_t j)
{
    if (i == 0) return !isRepeaterPowered ? 115 : 99;
    if (i == 1) return !isRepeaterPowered ? 131 : 147;
    return 5;
}

bool BlockRedstoneRepeater::shouldSideBeRendered(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k, int_t l)
{
    return l != 0 && l != 1;
}

int_t BlockRedstoneRepeater::getRenderType() { return 15; }

int_t BlockRedstoneRepeater::getBlockTextureFromSide(int_t i)
{
    return getBlockTextureFromSideAndMetadata(i, 0);
}

bool BlockRedstoneRepeater::isIndirectlyPoweringTo(World *world, int_t i, int_t j, int_t k, int_t l)
{
    return isPoweringTo(world, i, j, k, l);
}

bool BlockRedstoneRepeater::isPoweringTo(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k, int_t l)
{
    if (!isRepeaterPowered) return false;
    int_t i1 = iblockaccess->getBlockMetadata(i, j, k) & 3;
    if (i1 == 0 && l == 3) return true;
    if (i1 == 1 && l == 4) return true;
    if (i1 == 2 && l == 2) return true;
    return i1 == 3 && l == 5;
}

void BlockRedstoneRepeater::onNeighborBlockChange(World *world, int_t i, int_t j, int_t k, int_t l)
{
    if (!canBlockStay(world, i, j, k))
    {
        dropBlockAsItem(world, i, j, k, world->getBlockMetadata(i, j, k));
        world->setBlockWithNotify(i, j, k, 0);
        world->notifyBlocksOfNeighborChange(i + 1, j, k, blockID);
        world->notifyBlocksOfNeighborChange(i - 1, j, k, blockID);
        world->notifyBlocksOfNeighborChange(i, j, k + 1, blockID);
        world->notifyBlocksOfNeighborChange(i, j, k - 1, blockID);
        world->notifyBlocksOfNeighborChange(i, j - 1, k, blockID);
        world->notifyBlocksOfNeighborChange(i, j + 1, k, blockID);
        return;
    }
    int_t i1   = world->getBlockMetadata(i, j, k);
    bool flag  = getBlockTextureFromSideAndMetadata(world, i, j, k, i1);
    int_t j1   = (i1 & 0xc) >> 2;
    if (isRepeaterPowered && !flag)
        world->scheduleBlockUpdate(i, j, k, blockID, tickRate[j1] * 2);
    else if (!isRepeaterPowered && flag)
        world->scheduleBlockUpdate(i, j, k, blockID, tickRate[j1] * 2);
}

bool BlockRedstoneRepeater::getBlockTextureFromSideAndMetadata(World *world, int_t i, int_t j, int_t k, int_t l)
{
    int_t i1 = l & 3;
    switch (i1)
    {
    case 0:
        return world->isBlockIndirectlyProvidingPowerTo(i, j, k + 1, 3) ||
               (world->getBlockId(i, j, k + 1) == Block::redstoneWire->blockID && world->getBlockMetadata(i, j, k + 1) > 0);
    case 2:
        return world->isBlockIndirectlyProvidingPowerTo(i, j, k - 1, 2) ||
               (world->getBlockId(i, j, k - 1) == Block::redstoneWire->blockID && world->getBlockMetadata(i, j, k - 1) > 0);
    case 3:
        return world->isBlockIndirectlyProvidingPowerTo(i + 1, j, k, 5) ||
               (world->getBlockId(i + 1, j, k) == Block::redstoneWire->blockID && world->getBlockMetadata(i + 1, j, k) > 0);
    case 1:
        return world->isBlockIndirectlyProvidingPowerTo(i - 1, j, k, 4) ||
               (world->getBlockId(i - 1, j, k) == Block::redstoneWire->blockID && world->getBlockMetadata(i - 1, j, k) > 0);
    }
    return false;
}

bool BlockRedstoneRepeater::blockActivated(World *world, int_t i, int_t j, int_t k, EntityPlayer *entityplayer)
{
    int_t l  = world->getBlockMetadata(i, j, k);
    int_t i1 = (l & 0xc) >> 2;
    i1       = ((i1 + 1) << 2) & 0xc;
    world->setBlockMetadataWithNotify(i, j, k, i1 | (l & 3));
    return true;
}

bool BlockRedstoneRepeater::canProvidePower() { return true; }

void BlockRedstoneRepeater::onBlockPlacedBy(World *world, int_t i, int_t j, int_t k, EntityLiving *entityliving)
{
    int_t l = ((MathHelper::floor_double((double)((entityliving->rotationYaw * 4.0f) / 360.0f) + 0.5) & 3) + 2) % 4;
    world->setBlockMetadataWithNotify(i, j, k, l);
    bool flag = getBlockTextureFromSideAndMetadata(world, i, j, k, l);
    if (flag) world->scheduleBlockUpdate(i, j, k, blockID, 1);
}

void BlockRedstoneRepeater::onBlockAdded(World *world, int_t i, int_t j, int_t k)
{
    world->notifyBlocksOfNeighborChange(i + 1, j, k, blockID);
    world->notifyBlocksOfNeighborChange(i - 1, j, k, blockID);
    world->notifyBlocksOfNeighborChange(i, j, k + 1, blockID);
    world->notifyBlocksOfNeighborChange(i, j, k - 1, blockID);
    world->notifyBlocksOfNeighborChange(i, j - 1, k, blockID);
    world->notifyBlocksOfNeighborChange(i, j + 1, k, blockID);
}

void BlockRedstoneRepeater::onBlockDestroyedByPlayer(World *world, int_t i, int_t j, int_t k, int_t metadata)
{
    if (isRepeaterPowered)
    {
        world->notifyBlocksOfNeighborChange(i + 1, j, k, blockID);
        world->notifyBlocksOfNeighborChange(i - 1, j, k, blockID);
        world->notifyBlocksOfNeighborChange(i, j, k + 1, blockID);
        world->notifyBlocksOfNeighborChange(i, j, k - 1, blockID);
        world->notifyBlocksOfNeighborChange(i, j - 1, k, blockID);
        world->notifyBlocksOfNeighborChange(i, j + 1, k, blockID);
    }
    Block::onBlockDestroyedByPlayer(world, i, j, k, metadata);
}

bool  BlockRedstoneRepeater::isOpaqueCube() { return false; }

int_t BlockRedstoneRepeater::idDropped(int_t i, Random &random)
{
    return Item::redstoneRepeater->shiftedIndex;
}

void BlockRedstoneRepeater::randomDisplayTick(World *world, int_t i, int_t j, int_t k, Random &random)
{
    if (!isRepeaterPowered) return;
    int_t l  = world->getBlockMetadata(i, j, k);
    double d  = (double)((float)i + 0.5f) + (double)(random.nextFloat() - 0.5f) * 0.2;
    double d1 = (double)((float)j + 0.4f) + (double)(random.nextFloat() - 0.5f) * 0.2;
    double d2 = (double)((float)k + 0.5f) + (double)(random.nextFloat() - 0.5f) * 0.2;
    double d3 = 0.0;
    double d4 = 0.0;
    if (random.nextInt(2) == 0)
    {
        switch (l & 3)
        {
        case 0: d4 = -0.3125; break;
        case 2: d4 =  0.3125; break;
        case 3: d3 = -0.3125; break;
        case 1: d3 =  0.3125; break;
        }
    }
    else
    {
        int_t i1 = (l & 0xc) >> 2;
        switch (l & 3)
        {
        case 0: d4 =  isPowered[i1]; break;
        case 2: d4 = -isPowered[i1]; break;
        case 3: d3 =  isPowered[i1]; break;
        case 1: d3 = -isPowered[i1]; break;
        }
    }
    world->spawnParticle("reddust", d + d3, d1, d2 + d4, 0.0, 0.0, 0.0);
}
