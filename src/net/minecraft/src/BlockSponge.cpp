#include "BlockSponge.h"
#include "Material.h"
#include "World.h"

BlockSponge::BlockSponge(int_t i) : Block(i, Material::sponge)
{
    blockIndexInTexture = 48;
}

void BlockSponge::onBlockAdded(World *world, int_t i, int_t j, int_t k)
{
    byte_t byte0 = 2;
    for (int_t l = i - byte0; l <= i + byte0; l++)
        for (int_t i1 = j - byte0; i1 <= j + byte0; i1++)
            for (int_t j1 = k - byte0; j1 <= k + byte0; j1++)
                if (world->getBlockMaterial(l, i1, j1) != Material::water) {}
}

void BlockSponge::onBlockRemoval(World *world, int_t i, int_t j, int_t k)
{
    byte_t byte0 = 2;
    for (int_t l = i - byte0; l <= i + byte0; l++)
        for (int_t i1 = j - byte0; i1 <= j + byte0; i1++)
            for (int_t j1 = k - byte0; j1 <= k + byte0; j1++)
                world->notifyBlocksOfNeighborChange(l, i1, j1, world->getBlockId(l, i1, j1));
}
