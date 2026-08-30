#include "BlockBed.h"
#include "Material.h"
#include "World.h"
#include "Explosion.h"
#include "WorldProvider.h"
#include "EntityPlayer.h"
#include "ChunkCoordinates.h"
#include "EnumStatus.h"
#include "ModelBed.h"
#include "Item.h"
#include "IBlockAccess.h"
#include "Block.h"

const int_t BlockBed::headBlockToFootBlockMap[4][2] = {{0,1},{-1,0},{0,-1},{1,0}};

BlockBed::BlockBed(int_t i) : Block(i, 134, Material::cloth)
{
    setBounds();
}

bool BlockBed::blockActivated(World *world, int_t i, int_t j, int_t k, EntityPlayer *entityplayer)
{
    if (world->multiplayerWorld) return true;
    int_t l = world->getBlockMetadata(i, j, k);
    if (!isBlockFootOfBed(l))
    {
        int_t i1 = getDirectionFromMetadata(l);
        i += headBlockToFootBlockMap[i1][0];
        k += headBlockToFootBlockMap[i1][1];
        if (world->getBlockId(i, j, k) != blockID) return true;
        l = world->getBlockMetadata(i, j, k);
    }
    if (!world->worldProvider->canRespawnHere())
    {
        double d  = (double)i + 0.5;
        double d1 = (double)j + 0.5;
        double d2 = (double)k + 0.5;
        world->setBlockWithNotify(i, j, k, 0);
        int_t j1 = getDirectionFromMetadata(l);
        i += headBlockToFootBlockMap[j1][0];
        k += headBlockToFootBlockMap[j1][1];
        if (world->getBlockId(i, j, k) == blockID)
        {
            world->setBlockWithNotify(i, j, k, 0);
            d  = (d + (double)i + 0.5) / 2.0;
            d1 = (d1 + (double)j + 0.5) / 2.0;
            d2 = (d2 + (double)k + 0.5) / 2.0;
        }
        delete world->newExplosion(nullptr, (float)i + 0.5f, (float)j + 0.5f, (float)k + 0.5f, 5.0f, true);
        return true;
    }
    if (isBedOccupied(l))
    {
        EntityPlayer *entityplayer1 = nullptr;
        for (EntityPlayer *entityplayer2 : world->playerEntities)
        {
            if (entityplayer2->isPlayerSleeping())
            {
                ChunkCoordinates *chunkcoordinates = entityplayer2->bedChunkCoordinates;
                if (chunkcoordinates->x == i && chunkcoordinates->y == j && chunkcoordinates->z == k)
                    entityplayer1 = entityplayer2;
            }
        }
        if (entityplayer1 == nullptr)
            setBedOccupied(world, i, j, k, false);
        else
        {
            entityplayer->addChatMessage("tile.bed.occupied");
            return true;
        }
    }
    EnumStatus enumstatus = entityplayer->sleepInBedAt(i, j, k);
    if (enumstatus == EnumStatus::OK)
    {
        setBedOccupied(world, i, j, k, true);
        return true;
    }
    if (enumstatus == EnumStatus::NOT_POSSIBLE_NOW)
        entityplayer->addChatMessage("tile.bed.noSleep");
    else if (enumstatus == EnumStatus::NOT_SAFE)
        entityplayer->addChatMessage("tile.bed.notSafe");
    return true;
}

int_t BlockBed::getBlockTextureFromSideAndMetadata(int_t i, int_t j)
{
    if (i == 0) return Block::planks->blockIndexInTexture;
    int_t k = getDirectionFromMetadata(j);
    int_t l = ModelBed::bedDirection[k][i];
    if (isBlockFootOfBed(j))
    {
        if (l == 2) return blockIndexInTexture + 2 + 16;
        if (l == 5 || l == 4) return blockIndexInTexture + 1 + 16;
        return blockIndexInTexture + 1;
    }
    if (l == 3) return (blockIndexInTexture - 1) + 16;
    if (l == 5 || l == 4) return blockIndexInTexture + 16;
    return blockIndexInTexture;
}

int_t BlockBed::getRenderType()       { return 14; }
bool  BlockBed::renderAsNormalBlock() { return false; }
bool  BlockBed::isOpaqueCube()        { return false; }

void BlockBed::setBlockBoundsBasedOnState(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k)
{
    setBounds();
}

void BlockBed::onNeighborBlockChange(World *world, int_t i, int_t j, int_t k, int_t l)
{
    int_t i1 = world->getBlockMetadata(i, j, k);
    int_t j1 = getDirectionFromMetadata(i1);
    if (isBlockFootOfBed(i1))
    {
        if (world->getBlockId(i - headBlockToFootBlockMap[j1][0], j, k - headBlockToFootBlockMap[j1][1]) != blockID)
            world->setBlockWithNotify(i, j, k, 0);
    }
    else if (world->getBlockId(i + headBlockToFootBlockMap[j1][0], j, k + headBlockToFootBlockMap[j1][1]) != blockID)
    {
        world->setBlockWithNotify(i, j, k, 0);
        if (!world->multiplayerWorld)
            dropBlockAsItem(world, i, j, k, i1);
    }
}

int_t BlockBed::idDropped(int_t i, Random &random)
{
    if (isBlockFootOfBed(i)) return 0;
    return Item::bed->shiftedIndex;
}

void BlockBed::setBounds()
{
    setBlockBounds(0.0f, 0.0f, 0.0f, 1.0f, 0.5625f, 1.0f);
}

int_t BlockBed::getDirectionFromMetadata(int_t i) { return i & 3; }
bool  BlockBed::isBlockFootOfBed(int_t i)          { return (i & 8) != 0; }
bool  BlockBed::isBedOccupied(int_t i)             { return (i & 4) != 0; }

void BlockBed::setBedOccupied(World *world, int_t i, int_t j, int_t k, bool flag)
{
    int_t l = world->getBlockMetadata(i, j, k);
    if (flag) l |= 4; else l &= -5;
    world->setBlockMetadataWithNotify(i, j, k, l);
}

ChunkCoordinates *BlockBed::getNearestEmptyChunkCoordinates(World *world, int_t i, int_t j, int_t k, int_t l)
{
    int_t i1 = world->getBlockMetadata(i, j, k);
    int_t j1 = getDirectionFromMetadata(i1);
    for (int_t k1 = 0; k1 <= 1; k1++)
    {
        int_t l1 = i - headBlockToFootBlockMap[j1][0] * k1 - 1;
        int_t i2 = k - headBlockToFootBlockMap[j1][1] * k1 - 1;
        int_t j2 = l1 + 2;
        int_t k2 = i2 + 2;
        for (int_t l2 = l1; l2 <= j2; l2++)
        {
            for (int_t i3 = i2; i3 <= k2; i3++)
            {
                if (!world->isBlockNormalCube(l2, j - 1, i3) || !world->isAirBlock(l2, j, i3) || !world->isAirBlock(l2, j + 1, i3))
                    continue;
                if (l > 0)
                    l--;
                else
                    return new ChunkCoordinates(l2, j, i3);
            }
        }
    }
    return nullptr;
}

void BlockBed::dropBlockAsItemWithChance(World *world, int_t i, int_t j, int_t k, int_t l, float f)
{
    if (!isBlockFootOfBed(l))
        Block::dropBlockAsItemWithChance(world, i, j, k, l, f);
}

int_t BlockBed::getMobilityFlag() { return 1; }
