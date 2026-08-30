#include "BlockPistonExtension.h"
#include "Material.h"
#include "World.h"
#include "IBlockAccess.h"
#include "PistonBlockTextures.h"
#include "BlockPistonBase.h"
#include "Block.h"
#include "AxisAlignedBB.h"

BlockPistonExtension::BlockPistonExtension(int_t i, int_t j)
    : Block(i, j, Material::piston)
{
    pistonState = -1;
    setStepSound(soundStoneFootstep);
    setHardness(0.5f);
}

void BlockPistonExtension::getPistonExtensionState(int_t i) { pistonState = i; }
void BlockPistonExtension::getPistonExtensionFacing()          { pistonState = -1; }

void BlockPistonExtension::onBlockRemoval(World *world, int_t i, int_t j, int_t k)
{
    Block::onBlockRemoval(world, i, j, k);
    int_t l  = world->getBlockMetadata(i, j, k);
    int_t j1 = PistonBlockTextures::oppositeFacing[getPistonExtensionType(l)];
    i += PistonBlockTextures::deltaX[j1];
    j += PistonBlockTextures::deltaY[j1];
    k += PistonBlockTextures::deltaZ[j1];
    int_t k1 = world->getBlockId(i, j, k);
    if (k1 == Block::pistonBase->blockID || k1 == Block::pistonStickyBase->blockID)
    {
        int_t i1 = world->getBlockMetadata(i, j, k);
        if (BlockPistonBase::isPowered(i1))
        {
            Block::blocksList[k1]->dropBlockAsItem(world, i, j, k, i1);
            world->setBlockWithNotify(i, j, k, 0);
        }
    }
}

int_t BlockPistonExtension::getBlockTextureFromSideAndMetadata(int_t i, int_t j)
{
    int_t k = getPistonExtensionType(j);
    if (i == k)
    {
        if (pistonState >= 0)  return pistonState;
        if ((j & 8) != 0)        return blockIndexInTexture - 1;
        return blockIndexInTexture;
    }
    return i != PistonBlockTextures::oppositeFacing[k] ? 108 : 107;
}

int_t BlockPistonExtension::getRenderType()       { return 17; }
bool  BlockPistonExtension::isOpaqueCube()        { return false; }
bool  BlockPistonExtension::renderAsNormalBlock() { return false; }
bool  BlockPistonExtension::canPlaceBlockAt(World *world, int_t i, int_t j, int_t k) { return false; }
bool  BlockPistonExtension::canPlaceBlockOnSide(World *world, int_t i, int_t j, int_t k, int_t l) { return false; }
int_t BlockPistonExtension::quantityDropped(Random &random) { return 0; }

void BlockPistonExtension::getCollidingBoundingBoxes(World *world, int_t i, int_t j, int_t k,
                                                     AxisAlignedBB *axisalignedbb, std::vector<AxisAlignedBB*> &arraylist)
{
    int_t l = world->getBlockMetadata(i, j, k);
    switch (getPistonExtensionType(l))
    {
    case 0:
        setBlockBounds(0.0f, 0.0f, 0.0f, 1.0f, 0.25f, 1.0f);
        Block::getCollidingBoundingBoxes(world, i, j, k, axisalignedbb, arraylist);
        setBlockBounds(0.375f, 0.25f, 0.375f, 0.625f, 1.0f, 0.625f);
        Block::getCollidingBoundingBoxes(world, i, j, k, axisalignedbb, arraylist);
        break;
    case 1:
        setBlockBounds(0.0f, 0.75f, 0.0f, 1.0f, 1.0f, 1.0f);
        Block::getCollidingBoundingBoxes(world, i, j, k, axisalignedbb, arraylist);
        setBlockBounds(0.375f, 0.0f, 0.375f, 0.625f, 0.75f, 0.625f);
        Block::getCollidingBoundingBoxes(world, i, j, k, axisalignedbb, arraylist);
        break;
    case 2:
        setBlockBounds(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.25f);
        Block::getCollidingBoundingBoxes(world, i, j, k, axisalignedbb, arraylist);
        setBlockBounds(0.25f, 0.375f, 0.25f, 0.75f, 0.625f, 1.0f);
        Block::getCollidingBoundingBoxes(world, i, j, k, axisalignedbb, arraylist);
        break;
    case 3:
        setBlockBounds(0.0f, 0.0f, 0.75f, 1.0f, 1.0f, 1.0f);
        Block::getCollidingBoundingBoxes(world, i, j, k, axisalignedbb, arraylist);
        setBlockBounds(0.25f, 0.375f, 0.0f, 0.75f, 0.625f, 0.75f);
        Block::getCollidingBoundingBoxes(world, i, j, k, axisalignedbb, arraylist);
        break;
    case 4:
        setBlockBounds(0.0f, 0.0f, 0.0f, 0.25f, 1.0f, 1.0f);
        Block::getCollidingBoundingBoxes(world, i, j, k, axisalignedbb, arraylist);
        setBlockBounds(0.375f, 0.25f, 0.25f, 0.625f, 0.75f, 1.0f);
        Block::getCollidingBoundingBoxes(world, i, j, k, axisalignedbb, arraylist);
        break;
    case 5:
        setBlockBounds(0.75f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
        Block::getCollidingBoundingBoxes(world, i, j, k, axisalignedbb, arraylist);
        setBlockBounds(0.0f, 0.375f, 0.25f, 0.75f, 0.625f, 0.75f);
        Block::getCollidingBoundingBoxes(world, i, j, k, axisalignedbb, arraylist);
        break;
    }
    setBlockBounds(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
}

void BlockPistonExtension::setBlockBoundsBasedOnState(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k)
{
    int_t l = iblockaccess->getBlockMetadata(i, j, k);
    switch (getPistonExtensionType(l))
    {
    case 0: setBlockBounds(0.0f, 0.0f, 0.0f, 1.0f, 0.25f, 1.0f); break;
    case 1: setBlockBounds(0.0f, 0.75f, 0.0f, 1.0f, 1.0f, 1.0f); break;
    case 2: setBlockBounds(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.25f); break;
    case 3: setBlockBounds(0.0f, 0.0f, 0.75f, 1.0f, 1.0f, 1.0f); break;
    case 4: setBlockBounds(0.0f, 0.0f, 0.0f, 0.25f, 1.0f, 1.0f); break;
    case 5: setBlockBounds(0.75f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f); break;
    }
}

void BlockPistonExtension::onNeighborBlockChange(World *world, int_t i, int_t j, int_t k, int_t l)
{
    int_t i1 = getPistonExtensionType(world->getBlockMetadata(i, j, k));
    int_t j1 = world->getBlockId(
        i - PistonBlockTextures::deltaX[i1],
        j - PistonBlockTextures::deltaY[i1],
        k - PistonBlockTextures::deltaZ[i1]);
    if (j1 != Block::pistonBase->blockID && j1 != Block::pistonStickyBase->blockID)
        world->setBlockWithNotify(i, j, k, 0);
    else
        Block::blocksList[j1]->onNeighborBlockChange(world,
            i - PistonBlockTextures::deltaX[i1],
            j - PistonBlockTextures::deltaY[i1],
            k - PistonBlockTextures::deltaZ[i1], l);
}

int_t BlockPistonExtension::getPistonExtensionType(int_t i) { return i & 7; }

int_t BlockPistonExtension::getDirectionMeta(int_t metadata)
{
    return getPistonExtensionType(metadata);
}
