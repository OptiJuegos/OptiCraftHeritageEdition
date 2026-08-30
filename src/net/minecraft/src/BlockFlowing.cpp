#include "BlockFlowing.h"
#include "World.h"
#include "WorldProvider.h"
#include "Material.h"
#include "Block.h"

BlockFlowing::BlockFlowing(int_t i, Material *material) : BlockFluid(i, material)
{
    numAdjacentSources = 0;
    for (int_t n = 0; n < 4; n++) { isOptimalFlowDirection[n] = false; flowCost[n] = 0; }
}

void BlockFlowing::getFlowDirection(World *world, int_t i, int_t j, int_t k)
{
    int_t l = world->getBlockMetadata(i, j, k);
    world->setBlockAndMetadata(i, j, k, blockID + 1, l);
    world->markBlocksDirty(i, j, k, i, j, k);
    world->markBlockNeedsUpdate(i, j, k);
}

void BlockFlowing::updateTick(World *world, int_t i, int_t j, int_t k, Random &random)
{
    int_t l    = getFlowDecay(world, i, j, k);
    byte_t byte0 = 1;
    if (blockMaterial == Material::lava && !world->worldProvider->isHellWorld) byte0 = 2;
    bool flag = true;
    if (l > 0)
    {
        int_t i1 = -100;
        numAdjacentSources = 0;
        i1 = getSmallestFlowDecay(world, i - 1, j, k, i1);
        i1 = getSmallestFlowDecay(world, i + 1, j, k, i1);
        i1 = getSmallestFlowDecay(world, i, j, k - 1, i1);
        i1 = getSmallestFlowDecay(world, i, j, k + 1, i1);
        int_t j1 = i1 + byte0;
        if (j1 >= 8 || i1 < 0) j1 = -1;
        if (getFlowDecay(world, i, j + 1, k) >= 0)
        {
            int_t l1 = getFlowDecay(world, i, j + 1, k);
            j1 = (l1 >= 8) ? l1 : l1 + 8;
        }
        if (numAdjacentSources >= 2 && blockMaterial == Material::water)
        {
            if (world->getBlockMaterial(i, j - 1, k)->isSolid())
                j1 = 0;
            else if (world->getBlockMaterial(i, j - 1, k) == blockMaterial && world->getBlockMetadata(i, j, k) == 0)
                j1 = 0;
        }
        if (blockMaterial == Material::lava && l < 8 && j1 < 8 && j1 > l && random.nextInt(4) != 0)
        {
            j1 = l;
            flag = false;
        }
        if (j1 != l)
        {
            l = j1;
            if (l < 0)
                world->setBlockWithNotify(i, j, k, 0);
            else
            {
                world->setBlockMetadataWithNotify(i, j, k, l);
                world->scheduleBlockUpdate(i, j, k, blockID, tickRate());
                world->notifyBlocksOfNeighborChange(i, j, k, blockID);
            }
        }
        else if (flag)
        {
            getFlowDirection(world, i, j, k);
        }
    }
    else
    {
        getFlowDirection(world, i, j, k);
    }
    if (liquidCanDisplaceBlock(world, i, j - 1, k))
    {
        if (blockMaterial == Material::lava && world->getBlockMaterial(i, j - 1, k) == Material::water)
        {
            world->setBlockWithNotify(i, j - 1, k, Block::stone->blockID);
            triggerLavaMixEffects(world, i, j - 1, k);
            return;
        }
        if (l >= 8)
            world->setBlockAndMetadataWithNotify(i, j - 1, k, blockID, l);
        else
            world->setBlockAndMetadataWithNotify(i, j - 1, k, blockID, l + 8);
    }
    else if (l >= 0 && (l == 0 || blockBlocksFlow(world, i, j - 1, k)))
    {
        bool *aflag = getOptimalFlowDirections(world, i, j, k);
        int_t k1 = l + byte0;
        if (l >= 8) k1 = 1;
        if (k1 >= 8) return;
        if (aflag[0]) flowIntoBlock(world, i - 1, j, k, k1);
        if (aflag[1]) flowIntoBlock(world, i + 1, j, k, k1);
        if (aflag[2]) flowIntoBlock(world, i, j, k - 1, k1);
        if (aflag[3]) flowIntoBlock(world, i, j, k + 1, k1);
    }
}

void BlockFlowing::flowIntoBlock(World *world, int_t i, int_t j, int_t k, int_t l)
{
    if (liquidCanDisplaceBlock(world, i, j, k))
    {
        int_t i1 = world->getBlockId(i, j, k);
        if (i1 > 0)
        {
            if (blockMaterial == Material::lava)
                triggerLavaMixEffects(world, i, j, k);
            else
                Block::blocksList[i1]->dropBlockAsItem(world, i, j, k, world->getBlockMetadata(i, j, k));
        }
        world->setBlockAndMetadataWithNotify(i, j, k, blockID, l);
    }
}

int_t BlockFlowing::calculateFlowCost(World *world, int_t i, int_t j, int_t k, int_t l, int_t i1)
{
    int_t j1 = 1000;
    for (int_t k1 = 0; k1 < 4; k1++)
    {
        if ((k1 == 0 && i1 == 1) || (k1 == 1 && i1 == 0) || (k1 == 2 && i1 == 3) || (k1 == 3 && i1 == 2)) continue;
        int_t l1 = i, i2 = j, j2 = k;
        if (k1 == 0) l1--;
        if (k1 == 1) l1++;
        if (k1 == 2) j2--;
        if (k1 == 3) j2++;
        if (blockBlocksFlow(world, l1, i2, j2) ||
            (world->getBlockMaterial(l1, i2, j2) == blockMaterial && world->getBlockMetadata(l1, i2, j2) == 0)) continue;
        if (!blockBlocksFlow(world, l1, i2 - 1, j2)) return l;
        if (l >= 4) continue;
        int_t k2 = calculateFlowCost(world, l1, i2, j2, l + 1, k1);
        if (k2 < j1) j1 = k2;
    }
    return j1;
}

bool *BlockFlowing::getOptimalFlowDirections(World *world, int_t i, int_t j, int_t k)
{
    for (int_t l = 0; l < 4; l++)
    {
        flowCost[l] = 1000;
        int_t j1 = i, i2 = j, j2 = k;
        if (l == 0) j1--;
        if (l == 1) j1++;
        if (l == 2) j2--;
        if (l == 3) j2++;
        if (blockBlocksFlow(world, j1, i2, j2) ||
            (world->getBlockMaterial(j1, i2, j2) == blockMaterial && world->getBlockMetadata(j1, i2, j2) == 0)) continue;
        if (!blockBlocksFlow(world, j1, i2 - 1, j2))
            flowCost[l] = 0;
        else
            flowCost[l] = calculateFlowCost(world, j1, i2, j2, 1, l);
    }
    int_t i1 = flowCost[0];
    for (int_t k1 = 1; k1 < 4; k1++)
        if (flowCost[k1] < i1) i1 = flowCost[k1];
    for (int_t l1 = 0; l1 < 4; l1++)
        isOptimalFlowDirection[l1] = (flowCost[l1] == i1);
    return isOptimalFlowDirection;
}

bool BlockFlowing::blockBlocksFlow(World *world, int_t i, int_t j, int_t k)
{
    int_t l = world->getBlockId(i, j, k);
    if (l == Block::doorWood->blockID || l == Block::doorSteel->blockID ||
        l == Block::signPost->blockID || l == Block::ladder->blockID || l == Block::reed->blockID)
        return true;
    if (l == 0) return false;
    Material *material = Block::blocksList[l]->blockMaterial;
    return material == Material::portal || material->blocksMovement();
}

int_t BlockFlowing::getSmallestFlowDecay(World *world, int_t i, int_t j, int_t k, int_t l)
{
    int_t i1 = getFlowDecay(world, i, j, k);
    if (i1 < 0) return l;
    if (i1 == 0) numAdjacentSources++;
    if (i1 >= 8) i1 = 0;
    return (l >= 0 && i1 >= l) ? l : i1;
}

bool BlockFlowing::liquidCanDisplaceBlock(World *world, int_t i, int_t j, int_t k)
{
    Material *material = world->getBlockMaterial(i, j, k);
    if (material == blockMaterial) return false;
    if (material == Material::lava) return false;
    return !blockBlocksFlow(world, i, j, k);
}

void BlockFlowing::onBlockAdded(World *world, int_t i, int_t j, int_t k)
{
    BlockFluid::onBlockAdded(world, i, j, k);
    if (world->getBlockId(i, j, k) == blockID)
        world->scheduleBlockUpdate(i, j, k, blockID, tickRate());
}
