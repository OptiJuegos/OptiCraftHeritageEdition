#include "ItemMinecart.h"
#include "ItemStack.h"
#include "World.h"
#include "BlockRail.h"
#include "EntityMinecart.h"
#include "EntityPlayer.h"

ItemMinecart::ItemMinecart(int i, int j)
    : Item(i), minecartType(j) {
    maxStackSize = 1;
}

bool ItemMinecart::onItemUse(ItemStack* itemstack, EntityPlayer* entityplayer, World* world, int i, int j, int k, int l) {
    int i1 = world->getBlockId(i, j, k);
    if (BlockRail::isRailBlock(i1)) {
        if (!world->multiplayerWorld) {
            EntityMinecart *minecart = new EntityMinecart(world, (float)i + 0.5f, (float)j + 0.5f, (float)k + 0.5f, minecartType);
            if (!world->entityJoinedWorld(minecart))
                delete minecart;
        }
        itemstack->stackSize--;
        return true;
    } else {
        return false;
    }
}
