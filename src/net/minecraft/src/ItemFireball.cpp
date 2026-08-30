#include "ItemFireball.h"

#include "Block.h"
#include "BlockFire.h"
#include "EntityPlayer.h"
#include "ItemStack.h"
#include "PlayerCapabilities.h"
#include "World.h"

ItemFireball::ItemFireball(int id)
    : Item(id)
{
}

bool ItemFireball::onItemUse(ItemStack *stack, EntityPlayer *player, World *world, int x, int y, int z, int side)
{
    if (world->multiplayerWorld)
        return true;

    if (side == 0) --y;
    if (side == 1) ++y;
    if (side == 2) --z;
    if (side == 3) ++z;
    if (side == 4) --x;
    if (side == 5) ++x;

    if (!player->canPlayerEdit(x, y, z))
        return false;

    if (world->getBlockId(x, y, z) == 0)
    {
        world->playSoundEffect(static_cast<double>(x) + 0.5, static_cast<double>(y) + 0.5,
                               static_cast<double>(z) + 0.5, "fire.ignite", 1.0f,
                               itemRand.nextFloat() * 0.4f + 0.8f);
        world->setBlockWithNotify(x, y, z, Block::fire->blockID);
    }

    if (!player->capabilities.isCreativeMode)
        --stack->stackSize;
    return true;
}
