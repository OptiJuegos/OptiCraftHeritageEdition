#include "BlockDoor.h"
#include "Material.h"
#include "IBlockAccess.h"
#include "World.h"
#include "AxisAlignedBB.h"
#include "EntityPlayer.h"
#include "Vec3D.h"
#include "MovingObjectPosition.h"
#include "Block.h"
#include "Item.h"

BlockDoor::BlockDoor(int_t i, Material *material) : Block(i, material)
{
    blockIndexInTexture = 97;
    if (material == Material::iron)
        blockIndexInTexture++;
    float f  = 0.5f;
    float f1 = 1.0f;
    setBlockBounds(0.5f - f, 0.0f, 0.5f - f, 0.5f + f, f1, 0.5f + f);
}


int_t BlockDoor::getBlockTexture(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k, int_t l)
{
    if (l == 0 || l == 1)
        return blockIndexInTexture;

    int_t metadata = getFullMetadata(iblockaccess, i, j, k);
    int_t texture = blockIndexInTexture;
    if ((metadata & 8) != 0)
        texture -= 16;

    int_t direction = metadata & 3;
    bool open = (metadata & 4) != 0;
    if (!open)
    {
        if ((direction == 0 && l == 5) ||
            (direction == 1 && l == 3) ||
            (direction == 2 && l == 4) ||
            (direction == 3 && l == 2))
            texture = -texture;
        if ((metadata & 16) != 0)
            texture = -texture;
    }
    else if ((direction == 0 && l == 2) ||
             (direction == 1 && l == 5) ||
             (direction == 2 && l == 3) ||
             (direction == 3 && l == 4))
    {
        texture = -texture;
    }
    return texture;
}

int_t BlockDoor::getBlockTextureFromSideAndMetadata(int_t i, int_t j)
{
    if (i == 0 || i == 1) return blockIndexInTexture;
    int_t k = getState(j);
    if (((k == 0 || k == 2) ^ (i <= 3)))
        return blockIndexInTexture;
    int_t l = k / 2 + ((i & 1) ^ k);
    l += (j & 4) / 4;
    int_t i1 = blockIndexInTexture - (j & 8) * 2;
    if ((l & 1) != 0)
        i1 = -i1;
    return i1;
}

bool  BlockDoor::isOpaqueCube()        { return false; }
bool  BlockDoor::renderAsNormalBlock() { return false; }
int_t BlockDoor::getRenderType()       { return 7; }

AxisAlignedBB *BlockDoor::getSelectedBoundingBoxFromPool(World *world, int_t i, int_t j, int_t k)
{
    setBlockBoundsBasedOnState(world, i, j, k);
    return Block::getSelectedBoundingBoxFromPool(world, i, j, k);
}

AxisAlignedBB *BlockDoor::getCollisionBoundingBoxFromPool(World *world, int_t i, int_t j, int_t k)
{
    setBlockBoundsBasedOnState(world, i, j, k);
    return Block::getCollisionBoundingBoxFromPool(world, i, j, k);
}

void BlockDoor::setBlockBoundsBasedOnState(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k)
{
    setDoorRotation(getFullMetadata(iblockaccess, i, j, k));
}

int_t BlockDoor::getDoorOrientation(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k)
{
    return getFullMetadata(iblockaccess, i, j, k) & 3;
}

int_t BlockDoor::getFullMetadata(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k)
{
    int_t metadata = iblockaccess->getBlockMetadata(i, j, k);
    bool upperHalf = (metadata & 8) != 0;
    int_t lowerMetadata;
    int_t upperMetadata;
    if (upperHalf)
    {
        lowerMetadata = iblockaccess->getBlockMetadata(i, j - 1, k);
        upperMetadata = metadata;
    }
    else
    {
        lowerMetadata = metadata;
        upperMetadata = iblockaccess->getBlockMetadata(i, j + 1, k);
    }
    bool hingeRight = (upperMetadata & 1) != 0;
    return (lowerMetadata & 7) | (upperHalf ? 8 : 0) | (hingeRight ? 16 : 0);
}

void BlockDoor::setDoorRotation(int_t metadata)
{
    float thickness = 3.0f / 16.0f;
    setBlockBounds(0.0f, 0.0f, 0.0f, 1.0f, 2.0f, 1.0f);
    int_t direction = metadata & 3;
    bool open = (metadata & 4) != 0;
    bool hingeRight = (metadata & 16) != 0;
    if (direction == 0)
    {
        if (!open) setBlockBounds(0.0f, 0.0f, 0.0f, thickness, 1.0f, 1.0f);
        else if (!hingeRight) setBlockBounds(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, thickness);
        else setBlockBounds(0.0f, 0.0f, 1.0f - thickness, 1.0f, 1.0f, 1.0f);
    }
    else if (direction == 1)
    {
        if (!open) setBlockBounds(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, thickness);
        else if (!hingeRight) setBlockBounds(1.0f - thickness, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
        else setBlockBounds(0.0f, 0.0f, 0.0f, thickness, 1.0f, 1.0f);
    }
    else if (direction == 2)
    {
        if (!open) setBlockBounds(1.0f - thickness, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
        else if (!hingeRight) setBlockBounds(0.0f, 0.0f, 1.0f - thickness, 1.0f, 1.0f, 1.0f);
        else setBlockBounds(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, thickness);
    }
    else if (direction == 3)
    {
        if (!open) setBlockBounds(0.0f, 0.0f, 1.0f - thickness, 1.0f, 1.0f, 1.0f);
        else if (!hingeRight) setBlockBounds(0.0f, 0.0f, 0.0f, thickness, 1.0f, 1.0f);
        else setBlockBounds(1.0f - thickness, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
    }
}

void BlockDoor::onBlockClicked(World *world, int_t i, int_t j, int_t k, EntityPlayer *entityplayer)
{
    blockActivated(world, i, j, k, entityplayer);
}

bool BlockDoor::blockActivated(World *world, int_t i, int_t j, int_t k, EntityPlayer *entityplayer)
{
    if (blockMaterial == Material::iron)
        return true;

    int_t metadata = getFullMetadata(world, i, j, k);
    int_t toggled = (metadata & 7) ^ 4;
    if ((metadata & 8) != 0)
    {
        world->setBlockMetadataWithNotify(i, j - 1, k, toggled);
        world->markBlocksDirty(i, j - 1, k, i, j, k);
    }
    else
    {
        world->setBlockMetadataWithNotify(i, j, k, toggled);
        world->markBlocksDirty(i, j, k, i, j, k);
    }
    world->playAuxSFXAtEntity(entityplayer, 1003, i, j, k, 0);
    return true;
}

void BlockDoor::onPoweredBlockChange(World *world, int_t i, int_t j, int_t k, bool powered)
{
    int_t metadata = getFullMetadata(world, i, j, k);
    bool open = (metadata & 4) != 0;
    if (open == powered)
        return;

    int_t toggled = (metadata & 7) ^ 4;
    if ((metadata & 8) != 0)
    {
        world->setBlockMetadataWithNotify(i, j - 1, k, toggled);
        world->markBlocksDirty(i, j - 1, k, i, j, k);
    }
    else
    {
        world->setBlockMetadataWithNotify(i, j, k, toggled);
        world->markBlocksDirty(i, j, k, i, j, k);
    }
    world->playAuxSFXAtEntity(nullptr, 1003, i, j, k, 0);
}

void BlockDoor::onNeighborBlockChange(World *world, int_t i, int_t j, int_t k, int_t changedBlockId)
{
    int_t metadata = world->getBlockMetadata(i, j, k);
    if ((metadata & 8) != 0)
    {
        if (world->getBlockId(i, j - 1, k) != blockID)
            world->setBlockWithNotify(i, j, k, 0);
        if (changedBlockId > 0 && changedBlockId != blockID)
            onNeighborBlockChange(world, i, j - 1, k, changedBlockId);
        return;
    }

    bool removed = false;
    if (world->getBlockId(i, j + 1, k) != blockID)
    {
        world->setBlockWithNotify(i, j, k, 0);
        removed = true;
    }
    if (!world->isBlockNormalCube(i, j - 1, k))
    {
        world->setBlockWithNotify(i, j, k, 0);
        removed = true;
        if (world->getBlockId(i, j + 1, k) == blockID)
            world->setBlockWithNotify(i, j + 1, k, 0);
    }

    if (removed)
    {
        if (!world->multiplayerWorld)
            dropBlockAsItem(world, i, j, k, metadata);
        return;
    }

    bool powered = world->isBlockIndirectlyGettingPowered(i, j, k) ||
                   world->isBlockIndirectlyGettingPowered(i, j + 1, k);
    bool relevantChange = powered || changedBlockId == 0 ||
                          (changedBlockId > 0 && Block::blocksList[changedBlockId] != nullptr &&
                           Block::blocksList[changedBlockId]->canProvidePower());
    if (relevantChange && changedBlockId != blockID)
        onPoweredBlockChange(world, i, j, k, powered);
}

int_t BlockDoor::idDropped(int_t i, Random &random)
{
    if ((i & 8) != 0) return 0;
    if (blockMaterial == Material::iron) return Item::doorSteel->shiftedIndex;
    return Item::doorWood->shiftedIndex;
}

MovingObjectPosition *BlockDoor::collisionRayTrace(World *world, int_t i, int_t j, int_t k, Vec3D *vec3d, Vec3D *vec3d1)
{
    setBlockBoundsBasedOnState(world, i, j, k);
    return Block::collisionRayTrace(world, i, j, k, vec3d, vec3d1);
}

int_t BlockDoor::getState(int_t i)
{
    if ((i & 4) == 0) return (i - 1) & 3;
    return i & 3;
}

bool BlockDoor::canPlaceBlockAt(World *world, int_t i, int_t j, int_t k)
{
    if (j >= 255) return false;
    return world->isBlockNormalCube(i, j - 1, k) &&
           Block::canPlaceBlockAt(world, i, j, k) &&
           Block::canPlaceBlockAt(world, i, j + 1, k);
}

bool BlockDoor::isOpen(int_t i)
{
    return (i & 4) != 0;
}

int_t BlockDoor::getMobilityFlag() { return 1; }

bool BlockDoor::getBlocksMovement(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k)
{
    return (getFullMetadata(iblockaccess, i, j, k) & 4) != 0;
}
