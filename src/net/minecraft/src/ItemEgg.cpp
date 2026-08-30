#include "ItemEgg.h"
#include "ItemStack.h"
#include "World.h"
#include "EntityEgg.h"
#include "EntityPlayer.h"

ItemEgg::ItemEgg(int i)
    : Item(i) {
    maxStackSize = 16;
}

ItemStack* ItemEgg::onItemRightClick(ItemStack* itemstack, World* world, EntityPlayer* entityplayer) {
    if (!entityplayer->capabilities.isCreativeMode) {
        itemstack->stackSize--;
    }
    world->playSoundAtEntity(entityplayer, "random.bow", 0.5f, 0.4f / (itemRand.nextFloat() * 0.4f + 0.8f));
    if (!world->multiplayerWorld) {
        EntityEgg *egg = new EntityEgg(world, entityplayer);
        if (!world->entityJoinedWorld(egg))
            delete egg;
    }
    return itemstack;
}
