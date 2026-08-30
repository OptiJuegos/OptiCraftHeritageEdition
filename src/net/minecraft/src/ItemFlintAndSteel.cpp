#include "ItemFlintAndSteel.h"
#include "ItemStack.h"
#include "World.h"
#include "Block.h"
#include "BlockFire.h"
#include "EntityPlayer.h"

ItemFlintAndSteel::ItemFlintAndSteel(int i)
    : Item(i) {
    maxStackSize = 1;
    setMaxDamage(64);
}

bool ItemFlintAndSteel::onItemUse(ItemStack* itemstack, EntityPlayer* entityplayer, World* world, int i, int j, int k, int l) {
    if (l == 0) j--;
    if (l == 1) j++;
    if (l == 2) k--;
    if (l == 3) k++;
    if (l == 4) i--;
    if (l == 5) i++;
    if (!entityplayer->canPlayerEdit(i, j, k)) {
        return false;
    }
    int i1 = world->getBlockId(i, j, k);
    if (i1 == 0) {
        world->playSoundEffect((double)i + 0.5, (double)j + 0.5, (double)k + 0.5, "fire.ignite", 1.0f, itemRand.nextFloat() * 0.4f + 0.8f);
        world->setBlockWithNotify(i, j, k, Block::fire->blockID);
    }
    itemstack->damageItem(1, entityplayer);
    return true;
}
