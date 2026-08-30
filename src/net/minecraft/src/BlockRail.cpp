#include "BlockRail.h"
#include "Material.h"
#include "World.h"
#include "IBlockAccess.h"
#include "AxisAlignedBB.h"
#include "Vec3D.h"
#include "MovingObjectPosition.h"
#include "Block.h"
#include "RailLogic.h"

bool BlockRail::isRailBlockAt(World *world, int_t i, int_t j, int_t k)
{
    int_t l = world->getBlockId(i, j, k);
    return l == Block::rail->blockID || l == Block::railPowered->blockID || l == Block::railDetector->blockID;
}

bool BlockRail::isRailBlock(int_t i)
{
    return i == Block::rail->blockID || i == Block::railPowered->blockID || i == Block::railDetector->blockID;
}

BlockRail::BlockRail(int_t i, int_t j, bool flag) : Block(i, j, Material::circuits)
{
    isPowered = flag;
    setBlockBounds(0.0f, 0.0f, 0.0f, 1.0f, 0.125f, 1.0f);
}

bool BlockRail::getIsPowered() { return isPowered; }

AxisAlignedBB *BlockRail::getCollisionBoundingBoxFromPool(World *world, int_t i, int_t j, int_t k)
{
    return nullptr;
}

bool BlockRail::isOpaqueCube() { return false; }

MovingObjectPosition *BlockRail::collisionRayTrace(World *world, int_t i, int_t j, int_t k, Vec3D *vec3d, Vec3D *vec3d1)
{
    setBlockBoundsBasedOnState(world, i, j, k);
    return Block::collisionRayTrace(world, i, j, k, vec3d, vec3d1);
}

void BlockRail::setBlockBoundsBasedOnState(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k)
{
    int_t l = iblockaccess->getBlockMetadata(i, j, k);
    if (l >= 2 && l <= 5)
        setBlockBounds(0.0f, 0.0f, 0.0f, 1.0f, 0.625f, 1.0f);
    else
        setBlockBounds(0.0f, 0.0f, 0.0f, 1.0f, 0.125f, 1.0f);
}

int_t BlockRail::getBlockTextureFromSideAndMetadata(int_t i, int_t j)
{
    if (isPowered)
    {
        if (blockID == Block::railPowered->blockID && (j & 8) == 0)
            return blockIndexInTexture - 16;
    }
    else if (j >= 6)
    {
        return blockIndexInTexture - 16;
    }
    return blockIndexInTexture;
}

bool  BlockRail::renderAsNormalBlock() { return false; }
int_t BlockRail::getRenderType()       { return 9; }
int_t BlockRail::quantityDropped(Random &random) { return 1; }

bool BlockRail::canPlaceBlockAt(World *world, int_t i, int_t j, int_t k)
{
    return world->isBlockNormalCube(i, j - 1, k);
}

void BlockRail::onBlockAdded(World *world, int_t i, int_t j, int_t k)
{
    if (!world->multiplayerWorld)
    {
        getRailDirection(world, i, j, k, true);
        if (blockID == Block::railPowered->blockID)
            onNeighborBlockChange(world, i, j, k, blockID);
    }
}

void BlockRail::onNeighborBlockChange(World *world, int_t i, int_t j, int_t k, int_t l)
{
    if (world->multiplayerWorld) return;
    int_t i1 = world->getBlockMetadata(i, j, k);
    int_t j1 = i1;
    if (isPowered) j1 &= 7;
    bool flag = false;
    if (!world->isBlockNormalCube(i, j - 1, k)) flag = true;
    if (j1 == 2 && !world->isBlockNormalCube(i + 1, j, k)) flag = true;
    if (j1 == 3 && !world->isBlockNormalCube(i - 1, j, k)) flag = true;
    if (j1 == 4 && !world->isBlockNormalCube(i, j, k - 1)) flag = true;
    if (j1 == 5 && !world->isBlockNormalCube(i, j, k + 1)) flag = true;
    if (flag)
    {
        dropBlockAsItem(world, i, j, k, world->getBlockMetadata(i, j, k));
        world->setBlockWithNotify(i, j, k, 0);
    }
    else if (blockID == Block::railPowered->blockID)
    {
        bool flag1 = world->isBlockIndirectlyGettingPowered(i, j, k);
        flag1 = flag1 || getRailDirection(world, i, j, k, i1, true, 0) || getRailDirection(world, i, j, k, i1, false, 0);
        bool flag2 = false;
        if (flag1 && (i1 & 8) == 0)
        {
            world->setBlockMetadataWithNotify(i, j, k, j1 | 8);
            flag2 = true;
        }
        else if (!flag1 && (i1 & 8) != 0)
        {
            world->setBlockMetadataWithNotify(i, j, k, j1);
            flag2 = true;
        }
        if (flag2)
        {
            world->notifyBlocksOfNeighborChange(i, j - 1, k, blockID);
            if (j1 == 2 || j1 == 3 || j1 == 4 || j1 == 5)
                world->notifyBlocksOfNeighborChange(i, j + 1, k, blockID);
        }
    }
    else if (l > 0 && Block::blocksList[l]->canProvidePower() && !isPowered)
    {
        RailLogic logic(this, world, i, j, k);
        if (RailLogic::getNAdjacentTracks(&logic) == 3)
        {
            getRailDirection(world, i, j, k, false);
        }
    }
}

void BlockRail::getRailDirection(World *world, int_t i, int_t j, int_t k, bool flag)
{
    if (world->multiplayerWorld) return;
    RailLogic logic(this, world, i, j, k);
    logic.updateRail(world->isBlockIndirectlyGettingPowered(i, j, k), flag);
}

bool BlockRail::getRailDirection(World *world, int_t i, int_t j, int_t k, int_t l, bool flag, int_t i1)
{
    if (i1 >= 8) return false;
    int_t j1    = l & 7;
    bool flag1  = true;
    switch (j1)
    {
    case 0: if (flag) k++; else k--; break;
    case 1: if (flag) i--; else i++; break;
    case 2:
        if (flag) { i--; }
        else      { i++; j++; flag1 = false; }
        j1 = 1; break;
    case 3:
        if (flag) { i--; j++; flag1 = false; }
        else      { i++; }
        j1 = 1; break;
    case 4:
        if (flag) { k++; }
        else      { k--; j++; flag1 = false; }
        j1 = 0; break;
    case 5:
        if (flag) { k++; j++; flag1 = false; }
        else      { k--; }
        j1 = 0; break;
    }
    if (getRailMetadata(world, i, j, k, flag, i1, j1)) return true;
    return flag1 && getRailMetadata(world, i, j - 1, k, flag, i1, j1);
}

bool BlockRail::getRailMetadata(World *world, int_t i, int_t j, int_t k, bool flag, int_t l, int_t i1)
{
    int_t j1 = world->getBlockId(i, j, k);
    if (j1 == Block::railPowered->blockID)
    {
        int_t k1 = world->getBlockMetadata(i, j, k);
        int_t l1 = k1 & 7;
        if (i1 == 1 && (l1 == 0 || l1 == 4 || l1 == 5)) return false;
        if (i1 == 0 && (l1 == 1 || l1 == 2 || l1 == 3)) return false;
        if ((k1 & 8) != 0)
        {
            if (world->isBlockIndirectlyGettingPowered(i, j, k))
                return true;
            return getRailDirection(world, i, j, k, k1, flag, l + 1);
        }
    }
    return false;
}

int_t BlockRail::getMobilityFlag() { return 0; }

bool BlockRail::isPoweredBlockRail(BlockRail *blockrail) { return blockrail->isPowered; }
