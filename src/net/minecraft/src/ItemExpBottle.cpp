#include "ItemExpBottle.h"

#include "EntityExpBottle.h"
#include "EntityPlayer.h"
#include "ItemStack.h"
#include "PlayerCapabilities.h"
#include "World.h"

ItemExpBottle::ItemExpBottle(int id)
    : Item(id)
{
}

bool ItemExpBottle::hasEffect(ItemStack *)
{
    return true;
}

ItemStack *ItemExpBottle::onItemRightClick(ItemStack *stack, World *world, EntityPlayer *player)
{
    if (!player->capabilities.isCreativeMode)
        --stack->stackSize;

    world->playSoundAtEntity(player, "random.bow", 0.5f, 0.4f / (itemRand.nextFloat() * 0.4f + 0.8f));
    if (!world->multiplayerWorld)
    {
        EntityExpBottle *bottle = new EntityExpBottle(world, player);
        if (!world->spawnEntityInWorld(bottle))
            delete bottle;
    }
    return stack;
}
