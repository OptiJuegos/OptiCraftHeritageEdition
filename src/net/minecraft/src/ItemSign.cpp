#include "ItemSign.h"
#include "ItemStack.h"
#include "EntityPlayer.h"
#include "World.h"
#include "Block.h"
#include "MathHelper.h"
#include "TileEntitySign.h"
#include "Material.h"

ItemSign::ItemSign(int i)
    : Item(i) {
    maxStackSize = 1;
}

bool ItemSign::onItemUse(ItemStack* itemstack, EntityPlayer* entityplayer, World* world, int i, int j, int k, int l) {
    if (l == 0) {
        return false;
    }
    if (!world->getBlockMaterial(i, j, k)->isSolid()) {
        return false;
    }
    if (l == 1) {
        j++;
    }
    if (l == 2) {
        k--;
    }
    if (l == 3) {
        k++;
    }
    if (l == 4) {
        i--;
    }
    if (l == 5) {
        i++;
    }
    if (!entityplayer->canPlayerEdit(i, j, k)) {
        return false;
    }
    if (!Block::signPost->canPlaceBlockAt(world, i, j, k)) {
        return false;
    }
    if (l == 1) {
        world->setBlockAndMetadataWithNotify(i, j, k, Block::signPost->blockID,
            MathHelper::floor_double((double)(((entityplayer->rotationYaw + 180.0f) * 16.0f) / 360.0f) + 0.5) & 0xf);
    } else {
        world->setBlockAndMetadataWithNotify(i, j, k, Block::signWall->blockID, l);
    }
    itemstack->stackSize--;
    TileEntitySign* tileentitysign = (TileEntitySign*)world->getBlockTileEntity(i, j, k);
    if (tileentitysign != nullptr) {
        entityplayer->displayGUIEditSign(tileentitysign);
    }
    return true;
}
