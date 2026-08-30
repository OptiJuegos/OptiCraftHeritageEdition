#include "BlockPressurePlate.h"
#include "Material.h"
#include "World.h"
#include "IBlockAccess.h"
#include "AxisAlignedBB.h"
#include "Entity.h"
#include "EntityLiving.h"
#include "EntityPlayer.h"
#include "Block.h"

BlockPressurePlate::BlockPressurePlate(int_t i, int_t j, EnumMobType enummobtype, Material *material)
    : Block(i, j, material)
{
    triggerMobType = enummobtype;
    setTickOnLoad(true);
    float f = 0.0625f;
    setBlockBounds(f, 0.0f, f, 1.0f - f, 0.03125f, 1.0f - f);
}

int_t BlockPressurePlate::tickRate() { return 20; }

AxisAlignedBB *BlockPressurePlate::getCollisionBoundingBoxFromPool(World *world, int_t i, int_t j, int_t k)
{
    return nullptr;
}

bool BlockPressurePlate::isOpaqueCube()        { return false; }
bool BlockPressurePlate::renderAsNormalBlock()  { return false; }

bool BlockPressurePlate::canPlaceBlockAt(World *world, int_t i, int_t j, int_t k)
{
    return world->isBlockNormalCube(i, j - 1, k) ||
           world->getBlockId(i, j - 1, k) == Block::fence->blockID;
}

void BlockPressurePlate::onBlockAdded(World *world, int_t i, int_t j, int_t k)
{
}

void BlockPressurePlate::onNeighborBlockChange(World *world, int_t i, int_t j, int_t k, int_t l)
{
    bool flag = false;
    if (!world->isBlockNormalCube(i, j - 1, k) &&
        world->getBlockId(i, j - 1, k) != Block::fence->blockID)
        flag = true;
    if (flag)
    {
        dropBlockAsItem(world, i, j, k, world->getBlockMetadata(i, j, k));
        world->setBlockWithNotify(i, j, k, 0);
    }
}

void BlockPressurePlate::updateTick(World *world, int_t i, int_t j, int_t k, Random &random)
{
    if (world->multiplayerWorld) return;
    if (world->getBlockMetadata(i, j, k) == 0) return;
    setStateIfMobInteractsWithPlate(world, i, j, k);
}

void BlockPressurePlate::onEntityCollidedWithBlock(World *world, int_t i, int_t j, int_t k, Entity *entity)
{
    if (world->multiplayerWorld) return;
    if (world->getBlockMetadata(i, j, k) == 1) return;
    setStateIfMobInteractsWithPlate(world, i, j, k);
}

void BlockPressurePlate::setStateIfMobInteractsWithPlate(World *world, int_t i, int_t j, int_t k)
{
    bool flag  = world->getBlockMetadata(i, j, k) == 1;
    bool flag1 = false;
    float f    = 0.125f;
    AxisAlignedBB *aabb = AxisAlignedBB::getBoundingBoxFromPool(
        (float)i + f, j, (float)k + f,
        (float)(i + 1) - f, (double)j + 0.25, (float)(k + 1) - f);
    if (triggerMobType == EnumMobType::everything)
    {
        const auto& list = world->getEntitiesWithinAABBExcludingEntity(nullptr, aabb);
        if ((int_t)list.size() > 0) flag1 = true;
    }
    if (triggerMobType == EnumMobType::mobs)
    {
        const auto& list = world->getEntitiesWithinAABB(typeid(EntityLiving), aabb);
        if ((int_t)list.size() > 0) flag1 = true;
    }
    if (triggerMobType == EnumMobType::players)
    {
        const auto& list = world->getEntitiesWithinAABB(typeid(EntityPlayer), aabb);
        if ((int_t)list.size() > 0) flag1 = true;
    }
    if (flag1 && !flag)
    {
        world->setBlockMetadataWithNotify(i, j, k, 1);
        world->notifyBlocksOfNeighborChange(i, j, k, blockID);
        world->notifyBlocksOfNeighborChange(i, j - 1, k, blockID);
        world->markBlocksDirty(i, j, k, i, j, k);
        world->playSoundEffect((double)i + 0.5, (double)j + 0.1, (double)k + 0.5, "random.click", 0.3f, 0.6f);
    }
    if (!flag1 && flag)
    {
        world->setBlockMetadataWithNotify(i, j, k, 0);
        world->notifyBlocksOfNeighborChange(i, j, k, blockID);
        world->notifyBlocksOfNeighborChange(i, j - 1, k, blockID);
        world->markBlocksDirty(i, j, k, i, j, k);
        world->playSoundEffect((double)i + 0.5, (double)j + 0.1, (double)k + 0.5, "random.click", 0.3f, 0.5f);
    }
    if (flag1)
        world->scheduleBlockUpdate(i, j, k, blockID, tickRate());
}

void BlockPressurePlate::onBlockRemoval(World *world, int_t i, int_t j, int_t k)
{
    int_t l = world->getBlockMetadata(i, j, k);
    if (l > 0)
    {
        world->notifyBlocksOfNeighborChange(i, j, k, blockID);
        world->notifyBlocksOfNeighborChange(i, j - 1, k, blockID);
    }
    Block::onBlockRemoval(world, i, j, k);
}

void BlockPressurePlate::setBlockBoundsBasedOnState(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k)
{
    bool flag = iblockaccess->getBlockMetadata(i, j, k) == 1;
    float f   = 0.0625f;
    if (flag)
        setBlockBounds(f, 0.0f, f, 1.0f - f, 0.03125f, 1.0f - f);
    else
        setBlockBounds(f, 0.0f, f, 1.0f - f, 0.0625f,  1.0f - f);
}

bool BlockPressurePlate::isPoweringTo(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k, int_t l)
{
    return iblockaccess->getBlockMetadata(i, j, k) > 0;
}

bool BlockPressurePlate::isIndirectlyPoweringTo(World *world, int_t i, int_t j, int_t k, int_t l)
{
    if (world->getBlockMetadata(i, j, k) == 0) return false;
    return l == 1;
}

bool BlockPressurePlate::canProvidePower() { return true; }

void BlockPressurePlate::setBlockBoundsForItemRender()
{
    float f  = 0.5f;
    float f1 = 0.125f;
    float f2 = 0.5f;
    setBlockBounds(0.5f - f, 0.5f - f1, 0.5f - f2, 0.5f + f, 0.5f + f1, 0.5f + f2);
}

int_t BlockPressurePlate::getMobilityFlag() { return 1; }

bool BlockPressurePlate::getBlocksMovement(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k)
{
    (void)iblockaccess;
    (void)i;
    (void)j;
    (void)k;
    return true;
}
