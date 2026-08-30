#include "BlockSand.h"
#include "Material.h"
#include "World.h"
#include "EntityFallingSand.h"
#include "Block.h"
#include "BlockFire.h"

bool BlockSand::fallInstantly = false;

BlockSand::BlockSand(int_t i, int_t j) : Block(i, j, Material::sand)
{
}

void BlockSand::onBlockAdded(World *world, int_t i, int_t j, int_t k)
{
    world->scheduleBlockUpdate(i, j, k, blockID, tickRate());
}

void BlockSand::onNeighborBlockChange(World *world, int_t i, int_t j, int_t k, int_t l)
{
    world->scheduleBlockUpdate(i, j, k, blockID, tickRate());
}

void BlockSand::updateTick(World *world, int_t i, int_t j, int_t k, Random &random)
{
    tryToFall(world, i, j, k);
}

int_t BlockSand::tickRate()
{
    return 3;
}

bool BlockSand::canFallBelow(World *world, int_t i, int_t j, int_t k)
{
    int_t l = world->getBlockId(i, j, k);
    if (l == 0) return true;
    if (l == Block::fire->blockID) return true;
    Material *material = Block::blocksList[l]->blockMaterial;
    if (material == Material::water) return true;
    return material == Material::lava;
}

void BlockSand::tryToFall(World *world, int_t i, int_t j, int_t k)
{
    if (canFallBelow(world, i, j - 1, k) && j >= 0)
    {
        byte_t byte0 = 32;
        if (fallInstantly || !world->checkChunksExist(i - byte0, j - byte0, k - byte0, i + byte0, j + byte0, k + byte0))
        {
            world->setBlockWithNotify(i, j, k, 0);
            for (; canFallBelow(world, i, j - 1, k) && j > 0; j--) {}
            if (j > 0)
                world->setBlockWithNotify(i, j, k, blockID);
        }
        else if (!world->multiplayerWorld)
        {
            EntityFallingSand *e = new EntityFallingSand(world, (float)i + 0.5f, (float)j + 0.5f, (float)k + 0.5f, blockID);
            if (!world->entityJoinedWorld(e))
                delete e;
        }
    }
}
