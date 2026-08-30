#include "ItemFishingRod.h"
#include "ItemStack.h"
#include "EntityPlayer.h"
#include "EntityFish.h"
#include "World.h"

ItemFishingRod::ItemFishingRod(int i)
    : Item(i) {
    setMaxDamage(64);
    setMaxStackSize(1);
}

bool ItemFishingRod::isFull3D() {
    return true;
}

bool ItemFishingRod::shouldRotateAroundWhenRendering() {
    return true;
}

ItemStack* ItemFishingRod::onItemRightClick(ItemStack* itemstack, World* world, EntityPlayer* entityplayer) {
    if (entityplayer->fishEntity != nullptr) {
        int i = entityplayer->fishEntity->catchFish();
        itemstack->damageItem(i, entityplayer);
        entityplayer->swingItem();
    } else {
        world->playSoundAtEntity(entityplayer, "random.bow", 0.5f, 0.4f / (itemRand.nextFloat() * 0.4f + 0.8f));
        if (!world->multiplayerWorld) {
            EntityFish *fish = new EntityFish(world, entityplayer);
            if (!world->entityJoinedWorld(fish))
                delete fish;
        }
        entityplayer->swingItem();
    }
    return itemstack;
}
