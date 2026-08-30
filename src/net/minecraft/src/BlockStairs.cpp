#include "BlockStairs.h"
#include "Block.h"
#include "World.h"
#include "IBlockAccess.h"
#include "AxisAlignedBB.h"
#include "EntityLiving.h"
#include "EntityPlayer.h"
#include "Entity.h"
#include "Vec3D.h"
#include "MathHelper.h"

BlockStairs::BlockStairs(int_t i, Block *block)
    : Block(i, block->blockIndexInTexture, block->blockMaterial)
{
    modelBlock = block;
    setHardness(block->blockHardness);
    setResistance(block->blockResistance / 3.0f);
    setStepSound(block->stepSound);
    setLightOpacity(255);
}

void BlockStairs::setBlockBoundsBasedOnState(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k)
{
    setBlockBounds(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
}

AxisAlignedBB *BlockStairs::getCollisionBoundingBoxFromPool(World *world, int_t i, int_t j, int_t k)
{
    return Block::getCollisionBoundingBoxFromPool(world, i, j, k);
}

bool  BlockStairs::isOpaqueCube()        { return false; }
bool  BlockStairs::renderAsNormalBlock() { return false; }
int_t BlockStairs::getRenderType()       { return 10; }

bool BlockStairs::shouldSideBeRendered(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k, int_t l)
{
    return Block::shouldSideBeRendered(iblockaccess, i, j, k, l);
}

void BlockStairs::getCollidingBoundingBoxes(World *world, int_t i, int_t j, int_t k,
                                            AxisAlignedBB *axisalignedbb, std::vector<AxisAlignedBB*> &arraylist)
{
    int_t metadata = world->getBlockMetadata(i, j, k);
    int_t direction = metadata & 3;
    float lowerMinY = 0.0f;
    float lowerMaxY = 0.5f;
    float upperMinY = 0.5f;
    float upperMaxY = 1.0f;

    if ((metadata & 4) != 0)
    {
        lowerMinY = 0.5f;
        lowerMaxY = 1.0f;
        upperMinY = 0.0f;
        upperMaxY = 0.5f;
    }

    setBlockBounds(0.0f, lowerMinY, 0.0f, 1.0f, lowerMaxY, 1.0f);
    Block::getCollidingBoundingBoxes(world, i, j, k, axisalignedbb, arraylist);

    if (direction == 0)
    {
        setBlockBounds(0.5f, upperMinY, 0.0f, 1.0f, upperMaxY, 1.0f);
        Block::getCollidingBoundingBoxes(world, i, j, k, axisalignedbb, arraylist);
    }
    else if (direction == 1)
    {
        setBlockBounds(0.0f, upperMinY, 0.0f, 0.5f, upperMaxY, 1.0f);
        Block::getCollidingBoundingBoxes(world, i, j, k, axisalignedbb, arraylist);
    }
    else if (direction == 2)
    {
        setBlockBounds(0.0f, upperMinY, 0.5f, 1.0f, upperMaxY, 1.0f);
        Block::getCollidingBoundingBoxes(world, i, j, k, axisalignedbb, arraylist);
    }
    else if (direction == 3)
    {
        setBlockBounds(0.0f, upperMinY, 0.0f, 1.0f, upperMaxY, 0.5f);
        Block::getCollidingBoundingBoxes(world, i, j, k, axisalignedbb, arraylist);
    }

    setBlockBounds(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
}

void BlockStairs::randomDisplayTick(World *world, int_t i, int_t j, int_t k, Random &random)
{
    modelBlock->randomDisplayTick(world, i, j, k, random);
}

void BlockStairs::onBlockClicked(World *world, int_t i, int_t j, int_t k, EntityPlayer *entityplayer)
{
    modelBlock->onBlockClicked(world, i, j, k, entityplayer);
}

void BlockStairs::onBlockDestroyedByPlayer(World *world, int_t i, int_t j, int_t k, int_t l)
{
    modelBlock->onBlockDestroyedByPlayer(world, i, j, k, l);
}

int_t BlockStairs::getMixedBrightnessForBlock(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k)
{
    return modelBlock->getMixedBrightnessForBlock(iblockaccess, i, j, k);
}

float BlockStairs::getBlockBrightness(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k)
{
    return modelBlock->getBlockBrightness(iblockaccess, i, j, k);
}

float BlockStairs::getExplosionResistance(Entity *entity)
{
    return modelBlock->getExplosionResistance(entity);
}

int_t BlockStairs::getRenderBlockPass()
{
    return modelBlock->getRenderBlockPass();
}

int_t BlockStairs::idDropped(int_t i, Random &random)
{
    return modelBlock->idDropped(i, random);
}

int_t BlockStairs::quantityDropped(Random &random)
{
    return modelBlock->quantityDropped(random);
}

int_t BlockStairs::getBlockTextureFromSideAndMetadata(int_t i, int_t j)
{
    return modelBlock->getBlockTextureFromSideAndMetadata(i, 0);
}

int_t BlockStairs::getBlockTextureFromSide(int_t i)
{
    return modelBlock->getBlockTextureFromSideAndMetadata(i, 0);
}

int_t BlockStairs::getBlockTexture(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k, int_t l)
{
    return modelBlock->getBlockTexture(iblockaccess, i, j, k, l);
}

int_t BlockStairs::tickRate()
{
    return modelBlock->tickRate();
}

AxisAlignedBB *BlockStairs::getSelectedBoundingBoxFromPool(World *world, int_t i, int_t j, int_t k)
{
    return modelBlock->getSelectedBoundingBoxFromPool(world, i, j, k);
}

void BlockStairs::velocityToAddToEntity(World *world, int_t i, int_t j, int_t k, Entity *entity, Vec3D *vec3d)
{
    modelBlock->velocityToAddToEntity(world, i, j, k, entity, vec3d);
}

bool BlockStairs::isCollidable()
{
    return modelBlock->isCollidable();
}

bool BlockStairs::canCollideCheck(int_t i, bool flag)
{
    return modelBlock->canCollideCheck(i, flag);
}

bool BlockStairs::canPlaceBlockAt(World *world, int_t i, int_t j, int_t k)
{
    return modelBlock->canPlaceBlockAt(world, i, j, k);
}

void BlockStairs::onBlockAdded(World *world, int_t i, int_t j, int_t k)
{
    onNeighborBlockChange(world, i, j, k, 0);
    modelBlock->onBlockAdded(world, i, j, k);
}

void BlockStairs::onBlockRemoval(World *world, int_t i, int_t j, int_t k)
{
    modelBlock->onBlockRemoval(world, i, j, k);
}

void BlockStairs::dropBlockAsItemWithChance(World *world, int_t i, int_t j, int_t k, int_t l, float f)
{
    modelBlock->dropBlockAsItemWithChance(world, i, j, k, l, f);
}

void BlockStairs::onEntityWalking(World *world, int_t i, int_t j, int_t k, Entity *entity)
{
    modelBlock->onEntityWalking(world, i, j, k, entity);
}

void BlockStairs::updateTick(World *world, int_t i, int_t j, int_t k, Random &random)
{
    modelBlock->updateTick(world, i, j, k, random);
}

bool BlockStairs::blockActivated(World *world, int_t i, int_t j, int_t k, EntityPlayer *entityplayer)
{
    return modelBlock->blockActivated(world, i, j, k, entityplayer);
}

void BlockStairs::onBlockDestroyedByExplosion(World *world, int_t i, int_t j, int_t k)
{
    modelBlock->onBlockDestroyedByExplosion(world, i, j, k);
}

void BlockStairs::onBlockPlacedBy(World *world, int_t i, int_t j, int_t k, EntityLiving *entityliving)
{
    int_t l = MathHelper::floor_double((double)((entityliving->rotationYaw * 4.0f) / 360.0f) + 0.5) & 3;
    int_t upsideDown = world->getBlockMetadata(i, j, k) & 4;
    if (l == 0) world->setBlockMetadataWithNotify(i, j, k, 2 | upsideDown);
    if (l == 1) world->setBlockMetadataWithNotify(i, j, k, 1 | upsideDown);
    if (l == 2) world->setBlockMetadataWithNotify(i, j, k, 3 | upsideDown);
    if (l == 3) world->setBlockMetadataWithNotify(i, j, k, 0 | upsideDown);
}

void BlockStairs::onBlockPlaced(World *world, int_t i, int_t j, int_t k, int_t side)
{
    if (side == 0)
    {
        int_t metadata = world->getBlockMetadata(i, j, k);
        world->setBlockMetadataWithNotify(i, j, k, metadata | 4);
    }
}
