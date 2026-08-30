#include "BlockPortal.h"
#include "Material.h"
#include "IBlockAccess.h"
#include "World.h"
#include "Block.h"
#include "AxisAlignedBB.h"
#include "Entity.h"
#include "BlockFire.h"

BlockPortal::BlockPortal(int_t i, int_t j) : BlockBreakable(i, j, Material::portal, false)
{
}

AxisAlignedBB *BlockPortal::getCollisionBoundingBoxFromPool(World *world, int_t i, int_t j, int_t k)
{
    return nullptr;
}

void BlockPortal::setBlockBoundsBasedOnState(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k)
{
    if (iblockaccess->getBlockId(i - 1, j, k) == blockID || iblockaccess->getBlockId(i + 1, j, k) == blockID)
    {
        float f  = 0.5f;
        float f2 = 0.125f;
        setBlockBounds(0.5f - f, 0.0f, 0.5f - f2, 0.5f + f, 1.0f, 0.5f + f2);
    }
    else
    {
        float f1 = 0.125f;
        float f3 = 0.5f;
        setBlockBounds(0.5f - f1, 0.0f, 0.5f - f3, 0.5f + f1, 1.0f, 0.5f + f3);
    }
}

bool BlockPortal::isOpaqueCube()        { return false; }
bool BlockPortal::renderAsNormalBlock() { return false; }

bool BlockPortal::tryToCreatePortal(World *world, int_t i, int_t j, int_t k)
{
    int_t l  = 0;
    int_t i1 = 0;
    if (world->getBlockId(i - 1, j, k) == Block::obsidian->blockID ||
        world->getBlockId(i + 1, j, k) == Block::obsidian->blockID)
        l = 1;
    if (world->getBlockId(i, j, k - 1) == Block::obsidian->blockID ||
        world->getBlockId(i, j, k + 1) == Block::obsidian->blockID)
        i1 = 1;
    if (l == i1) return false;
    if (world->getBlockId(i - l, j, k - i1) == 0)
    {
        i -= l;
        k -= i1;
    }
    for (int_t j1 = -1; j1 <= 2; j1++)
    {
        for (int_t l1 = -1; l1 <= 3; l1++)
        {
            bool flag = j1 == -1 || j1 == 2 || l1 == -1 || l1 == 3;
            if ((j1 == -1 || j1 == 2) && (l1 == -1 || l1 == 3)) continue;
            int_t j2 = world->getBlockId(i + l * j1, j + l1, k + i1 * j1);
            if (flag)
            {
                if (j2 != Block::obsidian->blockID) return false;
                continue;
            }
            if (j2 != 0 && j2 != Block::fire->blockID) return false;
        }
    }
    world->editingBlocks = true;
    for (int_t k1 = 0; k1 < 2; k1++)
    {
        for (int_t i2 = 0; i2 < 3; i2++)
            world->setBlockWithNotify(i + l * k1, j + i2, k + i1 * k1, Block::portal->blockID);
    }
    world->editingBlocks = false;
    return true;
}

void BlockPortal::onNeighborBlockChange(World *world, int_t i, int_t j, int_t k, int_t l)
{
    int_t i1 = 0;
    int_t j1 = 1;
    if (world->getBlockId(i - 1, j, k) == blockID || world->getBlockId(i + 1, j, k) == blockID)
    {
        i1 = 1;
        j1 = 0;
    }
    int_t k1;
    for (k1 = j; world->getBlockId(i, k1 - 1, k) == blockID; k1--) {}
    if (world->getBlockId(i, k1 - 1, k) != Block::obsidian->blockID)
    {
        world->setBlockWithNotify(i, j, k, 0);
        return;
    }
    int_t l1;
    for (l1 = 1; l1 < 4 && world->getBlockId(i, k1 + l1, k) == blockID; l1++) {}
    if (l1 != 3 || world->getBlockId(i, k1 + l1, k) != Block::obsidian->blockID)
    {
        world->setBlockWithNotify(i, j, k, 0);
        return;
    }
    bool flag  = world->getBlockId(i - 1, j, k) == blockID || world->getBlockId(i + 1, j, k) == blockID;
    bool flag1 = world->getBlockId(i, j, k - 1) == blockID || world->getBlockId(i, j, k + 1) == blockID;
    if (flag && flag1)
    {
        world->setBlockWithNotify(i, j, k, 0);
        return;
    }
    if ((world->getBlockId(i + i1, j, k + j1) != Block::obsidian->blockID ||
         world->getBlockId(i - i1, j, k - j1) != blockID) &&
        (world->getBlockId(i - i1, j, k - j1) != Block::obsidian->blockID ||
         world->getBlockId(i + i1, j, k + j1) != blockID))
    {
        world->setBlockWithNotify(i, j, k, 0);
    }
}

bool BlockPortal::shouldSideBeRendered(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k, int_t l)
{
    if (iblockaccess->getBlockId(i, j, k) == blockID) return false;
    bool flag  = iblockaccess->getBlockId(i - 1, j, k) == blockID && iblockaccess->getBlockId(i - 2, j, k) != blockID;
    bool flag1 = iblockaccess->getBlockId(i + 1, j, k) == blockID && iblockaccess->getBlockId(i + 2, j, k) != blockID;
    bool flag2 = iblockaccess->getBlockId(i, j, k - 1) == blockID && iblockaccess->getBlockId(i, j, k - 2) != blockID;
    bool flag3 = iblockaccess->getBlockId(i, j, k + 1) == blockID && iblockaccess->getBlockId(i, j, k + 2) != blockID;
    bool flag4 = flag  || flag1;
    bool flag5 = flag2 || flag3;
    if (flag4 && l == 4) return true;
    if (flag4 && l == 5) return true;
    if (flag5 && l == 2) return true;
    return flag5 && l == 3;
}

int_t BlockPortal::quantityDropped(Random &random) { return 0; }
int_t BlockPortal::getRenderBlockPass()             { return 1; }

void BlockPortal::onEntityCollidedWithBlock(World *world, int_t i, int_t j, int_t k, Entity *entity)
{
    if (entity->ridingEntity == nullptr && entity->riddenByEntity == nullptr)
        entity->setInPortal();
}

void BlockPortal::randomDisplayTick(World *world, int_t i, int_t j, int_t k, Random &random)
{
    if (random.nextInt(100) == 0)
    {
        world->playSoundEffect((double)i + 0.5, (double)j + 0.5, (double)k + 0.5,
                               "portal.portal", 0.5f, random.nextFloat() * 0.4f + 0.8f);
    }
    for (int_t l = 0; l < 4; l++)
    {
        double d  = (float)i + random.nextFloat();
        double d1 = (float)j + random.nextFloat();
        double d2 = (float)k + random.nextFloat();
        double d3 = 0.0;
        double d4 = 0.0;
        double d5 = 0.0;
        int_t  i1 = random.nextInt(2) * 2 - 1;
        d3 = ((double)random.nextFloat() - 0.5) * 0.5;
        d4 = ((double)random.nextFloat() - 0.5) * 0.5;
        d5 = ((double)random.nextFloat() - 0.5) * 0.5;
        if (world->getBlockId(i - 1, j, k) == blockID || world->getBlockId(i + 1, j, k) == blockID)
        {
            d2 = (double)k + 0.5 + 0.25 * (double)i1;
            d5 = random.nextFloat() * 2.0f * (float)i1;
        }
        else
        {
            d  = (double)i + 0.5 + 0.25 * (double)i1;
            d3 = random.nextFloat() * 2.0f * (float)i1;
        }
        world->spawnParticle("portal", d, d1, d2, d3, d4, d5);
    }
}
