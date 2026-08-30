#include "BlockVine.h"

#include <algorithm>

#include "BiomeGenBase.h"
#include "ColorizerFoliage.h"
#include "Config.h"
#include "EntityPlayer.h"
#include "IBlockAccess.h"
#include "Item.h"
#include "ItemStack.h"
#include "Material.h"
#include "StatList.h"
#include "World.h"
#include "WorldChunkManager.h"

namespace
{
    constexpr int_t kOffsetX[4] = {0, -1, 0, 1};
    constexpr int_t kOffsetZ[4] = {1, 0, -1, 0};
    constexpr int_t kVineGrowth[6] = {-1, -1, 2, 0, 1, 3};
}

BlockVine::BlockVine(int_t id)
    : Block(id, 143, Material::vine)
{
    setTickOnLoad(true);
}

void BlockVine::setBlockBoundsForItemRender()
{
    setBlockBounds(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
}

int_t BlockVine::getRenderType()
{
    return 20;
}

bool BlockVine::isOpaqueCube()
{
    return false;
}

bool BlockVine::renderAsNormalBlock()
{
    return false;
}

void BlockVine::setBlockBoundsBasedOnState(IBlockAccess *blockAccess, int_t x, int_t y, int_t z)
{
    const int_t metadata = blockAccess->getBlockMetadata(x, y, z);
    float minXValue = 1.0f;
    float minYValue = 1.0f;
    float minZValue = 1.0f;
    float maxXValue = 0.0f;
    float maxYValue = 0.0f;
    float maxZValue = 0.0f;
    bool hasSide = metadata > 0;

    if ((metadata & 2) != 0)
    {
        maxXValue = std::max(maxXValue, 1.0f / 16.0f);
        minXValue = 0.0f;
        minYValue = 0.0f;
        maxYValue = 1.0f;
        minZValue = 0.0f;
        maxZValue = 1.0f;
        hasSide = true;
    }
    if ((metadata & 8) != 0)
    {
        minXValue = std::min(minXValue, 15.0f / 16.0f);
        maxXValue = 1.0f;
        minYValue = 0.0f;
        maxYValue = 1.0f;
        minZValue = 0.0f;
        maxZValue = 1.0f;
        hasSide = true;
    }
    if ((metadata & 4) != 0)
    {
        maxZValue = std::max(maxZValue, 1.0f / 16.0f);
        minZValue = 0.0f;
        minXValue = 0.0f;
        maxXValue = 1.0f;
        minYValue = 0.0f;
        maxYValue = 1.0f;
        hasSide = true;
    }
    if ((metadata & 1) != 0)
    {
        minZValue = std::min(minZValue, 15.0f / 16.0f);
        maxZValue = 1.0f;
        minXValue = 0.0f;
        maxXValue = 1.0f;
        minYValue = 0.0f;
        maxYValue = 1.0f;
        hasSide = true;
    }

    if (!hasSide && canBePlacedOn(blockAccess->getBlockId(x, y + 1, z)))
    {
        minYValue = std::min(minYValue, 15.0f / 16.0f);
        maxYValue = 1.0f;
        minXValue = 0.0f;
        maxXValue = 1.0f;
        minZValue = 0.0f;
        maxZValue = 1.0f;
    }

    setBlockBounds(minXValue, minYValue, minZValue, maxXValue, maxYValue, maxZValue);
}

AxisAlignedBB *BlockVine::getCollisionBoundingBoxFromPool(World *world, int_t x, int_t y, int_t z)
{
    return nullptr;
}

bool BlockVine::canPlaceBlockOnSide(World *world, int_t x, int_t y, int_t z, int_t side)
{
    switch (side)
    {
        case 1: return canBePlacedOn(world->getBlockId(x, y + 1, z));
        case 2: return canBePlacedOn(world->getBlockId(x, y, z + 1));
        case 3: return canBePlacedOn(world->getBlockId(x, y, z - 1));
        case 4: return canBePlacedOn(world->getBlockId(x + 1, y, z));
        case 5: return canBePlacedOn(world->getBlockId(x - 1, y, z));
        default: return false;
    }
}

bool BlockVine::canBePlacedOn(int_t blockId) const
{
    if (blockId <= 0 || blockId >= BLOCK_REGISTRY_SIZE)
        return false;
    Block *block = Block::blocksList[blockId];
    return block != nullptr && block->renderAsNormalBlock() && block->blockMaterial->getIsSolid();
}

bool BlockVine::canVineStay(World *world, int_t x, int_t y, int_t z)
{
    const int_t metadata = world->getBlockMetadata(x, y, z);
    int_t validMetadata = metadata;

    if (metadata > 0)
    {
        for (int_t direction = 0; direction <= 3; ++direction)
        {
            const int_t bit = 1 << direction;
            if ((metadata & bit) != 0 &&
                !canBePlacedOn(world->getBlockId(x + kOffsetX[direction], y, z + kOffsetZ[direction])) &&
                (world->getBlockId(x, y + 1, z) != blockID ||
                 (world->getBlockMetadata(x, y + 1, z) & bit) == 0))
            {
                validMetadata &= ~bit;
            }
        }
    }

    if (validMetadata == 0 && !canBePlacedOn(world->getBlockId(x, y + 1, z)))
        return false;

    if (validMetadata != metadata)
        world->setBlockMetadataWithNotify(x, y, z, validMetadata);
    return true;
}

int_t BlockVine::getBlockColor()
{
    return ColorizerFoliage::getFoliageColorBasic();
}

int_t BlockVine::getRenderColor(int_t metadata)
{
    return ColorizerFoliage::getFoliageColorBasic();
}

int_t BlockVine::colorMultiplier(IBlockAccess *blockAccess, int_t x, int_t y, int_t z)
{
    WorldChunkManager *manager = blockAccess->getWorldChunkManager();
    if (manager == nullptr)
        return ColorizerFoliage::getFoliageColorBasic();

    if (!Config::isSmoothBiomes())
    {
        BiomeGenBase *biome = manager->getBiomeGenAt(x, z);
        return biome != nullptr ? biome->getBiomeFoliageColor() : ColorizerFoliage::getFoliageColorBasic();
    }

    int_t red = 0;
    int_t green = 0;
    int_t blue = 0;
    int_t samples = 0;
    for (int_t dz = -1; dz <= 1; ++dz)
    {
        for (int_t dx = -1; dx <= 1; ++dx)
        {
            BiomeGenBase *biome = manager->getBiomeGenAt(x + dx, z + dz);
            if (biome == nullptr)
                continue;
            const int_t color = biome->getBiomeFoliageColor();
            red += color >> 16 & 255;
            green += color >> 8 & 255;
            blue += color & 255;
            ++samples;
        }
    }

    if (samples == 0)
        return ColorizerFoliage::getFoliageColorBasic();
    return (red / samples & 255) << 16 | (green / samples & 255) << 8 | blue / samples & 255;
}

void BlockVine::onNeighborBlockChange(World *world, int_t x, int_t y, int_t z, int_t neighborId)
{
    if (!world->multiplayerWorld && !canVineStay(world, x, y, z))
    {
        dropBlockAsItem(world, x, y, z, world->getBlockMetadata(x, y, z));
        world->setBlockWithNotify(x, y, z, 0);
    }
}

void BlockVine::updateTick(World *world, int_t x, int_t y, int_t z, Random &random)
{
    if (world->multiplayerWorld || world->rand.nextInt(4) != 0)
        return;

    constexpr int_t radius = 4;
    int_t remaining = 5;
    bool tooDense = false;
    for (int_t checkX = x - radius; checkX <= x + radius && !tooDense; ++checkX)
    {
        for (int_t checkZ = z - radius; checkZ <= z + radius && !tooDense; ++checkZ)
        {
            for (int_t checkY = y - 1; checkY <= y + 1; ++checkY)
            {
                if (world->getBlockId(checkX, checkY, checkZ) == blockID && --remaining <= 0)
                {
                    tooDense = true;
                    break;
                }
            }
        }
    }

    const int_t metadata = world->getBlockMetadata(x, y, z);
    const int_t side = world->rand.nextInt(6);
    const int_t direction = kVineGrowth[side];

    if (side == 1 && y < 255 && world->isAirBlock(x, y + 1, z))
    {
        if (tooDense)
            return;
        int_t upperMetadata = world->rand.nextInt(16) & metadata;
        if (upperMetadata > 0)
        {
            for (int_t dir = 0; dir <= 3; ++dir)
            {
                if (!canBePlacedOn(world->getBlockId(x + kOffsetX[dir], y + 1, z + kOffsetZ[dir])))
                    upperMetadata &= ~(1 << dir);
            }
            if (upperMetadata > 0)
                world->setBlockAndMetadataWithNotify(x, y + 1, z, blockID, upperMetadata);
        }
        return;
    }

    if (side >= 2 && side <= 5 && (metadata & (1 << direction)) == 0)
    {
        if (tooDense)
            return;

        const int_t targetX = x + kOffsetX[direction];
        const int_t targetZ = z + kOffsetZ[direction];
        const int_t targetId = world->getBlockId(targetX, y, targetZ);
        if (targetId != 0)
        {
            Block *target = targetId >= 0 && targetId < BLOCK_REGISTRY_SIZE ? Block::blocksList[targetId] : nullptr;
            if (target != nullptr && target->blockMaterial->getIsTranslucent() && target->renderAsNormalBlock())
                world->setBlockMetadataWithNotify(x, y, z, metadata | (1 << direction));
            return;
        }

        const int_t leftDirection = (direction + 1) & 3;
        const int_t rightDirection = (direction + 3) & 3;
        if ((metadata & (1 << leftDirection)) != 0 &&
            canBePlacedOn(world->getBlockId(targetX + kOffsetX[leftDirection], y, targetZ + kOffsetZ[leftDirection])))
        {
            world->setBlockAndMetadataWithNotify(targetX, y, targetZ, blockID, 1 << leftDirection);
        }
        else if ((metadata & (1 << rightDirection)) != 0 &&
                 canBePlacedOn(world->getBlockId(targetX + kOffsetX[rightDirection], y, targetZ + kOffsetZ[rightDirection])))
        {
            world->setBlockAndMetadataWithNotify(targetX, y, targetZ, blockID, 1 << rightDirection);
        }
        else if ((metadata & (1 << leftDirection)) != 0 &&
                 world->isAirBlock(targetX + kOffsetX[leftDirection], y, targetZ + kOffsetZ[leftDirection]) &&
                 canBePlacedOn(world->getBlockId(x + kOffsetX[leftDirection], y, z + kOffsetZ[leftDirection])))
        {
            world->setBlockAndMetadataWithNotify(targetX + kOffsetX[leftDirection], y,
                                                 targetZ + kOffsetZ[leftDirection], blockID,
                                                 1 << ((direction + 2) & 3));
        }
        else if ((metadata & (1 << rightDirection)) != 0 &&
                 world->isAirBlock(targetX + kOffsetX[rightDirection], y, targetZ + kOffsetZ[rightDirection]) &&
                 canBePlacedOn(world->getBlockId(x + kOffsetX[rightDirection], y, z + kOffsetZ[rightDirection])))
        {
            world->setBlockAndMetadataWithNotify(targetX + kOffsetX[rightDirection], y,
                                                 targetZ + kOffsetZ[rightDirection], blockID,
                                                 1 << ((direction + 2) & 3));
        }
        else if (canBePlacedOn(world->getBlockId(targetX, y + 1, targetZ)))
        {
            world->setBlockAndMetadataWithNotify(targetX, y, targetZ, blockID, 0);
        }
        return;
    }

    if (y <= 1)
        return;

    const int_t belowId = world->getBlockId(x, y - 1, z);
    if (belowId == 0)
    {
        const int_t lowerMetadata = world->rand.nextInt(16) & metadata;
        if (lowerMetadata > 0)
            world->setBlockAndMetadataWithNotify(x, y - 1, z, blockID, lowerMetadata);
    }
    else if (belowId == blockID)
    {
        const int_t lowerMetadata = world->rand.nextInt(16) & metadata;
        const int_t oldBelowMetadata = world->getBlockMetadata(x, y - 1, z);
        if (oldBelowMetadata != (oldBelowMetadata | lowerMetadata))
            world->setBlockMetadataWithNotify(x, y - 1, z, oldBelowMetadata | lowerMetadata);
    }
}

void BlockVine::onBlockPlaced(World *world, int_t x, int_t y, int_t z, int_t side)
{
    int_t metadata = 0;
    switch (side)
    {
        case 2: metadata = 1; break;
        case 3: metadata = 4; break;
        case 4: metadata = 8; break;
        case 5: metadata = 2; break;
        default: break;
    }
    if (metadata != 0)
        world->setBlockMetadataWithNotify(x, y, z, metadata);
}

int_t BlockVine::idDropped(int_t metadata, Random &random)
{
    return 0;
}

int_t BlockVine::quantityDropped(Random &random)
{
    return 0;
}

void BlockVine::harvestBlock(World *world, EntityPlayer *player, int_t x, int_t y, int_t z, int_t metadata)
{
    ItemStack *equipped = player != nullptr ? player->getCurrentEquippedItem() : nullptr;
    if (!world->multiplayerWorld && equipped != nullptr && equipped->itemID == Item::shears->shiftedIndex)
    {
        if (blockID >= 0 && blockID < 256 && StatList::mineBlockStatArray[blockID] != nullptr)
            player->addStat(StatList::mineBlockStatArray[blockID], 1);
        dropBlockAsItem_do(world, x, y, z, new ItemStack(this, 1, 0));
        return;
    }
    Block::harvestBlock(world, player, x, y, z, metadata);
}
