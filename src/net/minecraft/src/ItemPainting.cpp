#include "ItemPainting.h"
#include "ItemStack.h"
#include "EntityPainting.h"
#include "World.h"
#include "EntityPlayer.h"

ItemPainting::ItemPainting(int i)
    : Item(i) {
}

bool ItemPainting::onItemUse(ItemStack* itemstack, EntityPlayer* entityplayer, World* world, int i, int j, int k, int l) {
    if (l == 0) return false;
    if (l == 1) return false;
    char byte0 = 0;
    if (l == 4) byte0 = 1;
    if (l == 3) byte0 = 2;
    if (l == 5) byte0 = 3;
    if (!entityplayer->canPlayerEdit(i, j, k)) {
        return false;
    }
    EntityPainting* entitypainting = new EntityPainting(world, i, j, k, byte0);
    if (entitypainting->onValidSurface()) {
        if (!world->multiplayerWorld && world->entityJoinedWorld(entitypainting)) {
            entitypainting = nullptr;
        }
        itemstack->stackSize--;
    }
    delete entitypainting;
    return true;
}
