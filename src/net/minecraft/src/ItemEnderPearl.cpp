#include "ItemEnderPearl.h"

#include "EntityEnderPearl.h"
#include "EntityPlayer.h"
#include "ItemStack.h"
#include "PlayerCapabilities.h"
#include "World.h"

ItemEnderPearl::ItemEnderPearl(int id)
    : Item(id)
{
    maxStackSize = 16;
}

ItemStack *ItemEnderPearl::onItemRightClick(ItemStack *stack, World *world, EntityPlayer *player)
{
    if (player->capabilities.isCreativeMode || player->ridingEntity != nullptr)
        return stack;

    --stack->stackSize;
    world->playSoundAtEntity(player, "random.bow", 0.5f, 0.4f / (itemRand.nextFloat() * 0.4f + 0.8f));
    if (!world->multiplayerWorld)
    {
        EntityEnderPearl *pearl = new EntityEnderPearl(world, player);
        if (!world->spawnEntityInWorld(pearl))
            delete pearl;
    }
    return stack;
}
