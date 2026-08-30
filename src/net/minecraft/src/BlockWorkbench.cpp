#include "BlockWorkbench.h"
#include "Material.h"
#include "World.h"
#include "EntityPlayer.h"
#include "Block.h"

BlockWorkbench::BlockWorkbench(int_t i) : Block(i, Material::wood)
{
    blockIndexInTexture = 59;
}

int_t BlockWorkbench::getBlockTextureFromSide(int_t i)
{
    if (i == 1) return blockIndexInTexture - 16;
    if (i == 0) return Block::planks->getBlockTextureFromSide(0);
    if (i == 2 || i == 4) return blockIndexInTexture + 1;
    return blockIndexInTexture;
}

bool BlockWorkbench::blockActivated(World *world, int_t i, int_t j, int_t k, EntityPlayer *entityplayer)
{
    if (world->multiplayerWorld) return true;
    entityplayer->displayWorkbenchGUI(i, j, k);
    return true;
}
