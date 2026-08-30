#include "ItemHoe.h"
#include "ItemStack.h"
#include "World.h"
#include "Block.h"
#include "StepSound.h"
#include "EntityPlayer.h"
#include "BlockGrass.h"

ItemHoe::ItemHoe(int i, EnumToolMaterial enumtoolmaterial)
    : Item(i) {
    maxStackSize = 1;
    setMaxDamage(EnumToolMaterialHelper::getMaxUses(enumtoolmaterial));
}

bool ItemHoe::onItemUse(ItemStack* itemstack, EntityPlayer* entityplayer, World* world, int i, int j, int k, int l) {
    int i1 = world->getBlockId(i, j, k);
    int j1 = world->getBlockId(i, j + 1, k);
    if (l != 0 && j1 == 0 && (i1 == Block::grass->blockID || i1 == Block::dirt->blockID)) {
        Block* block = Block::tilledField;
        world->playSoundEffect((float)i + 0.5f, (float)j + 0.5f, (float)k + 0.5f, block->stepSound->getStepSound(), (block->stepSound->getVolume() + 1.0f) / 2.0f, block->stepSound->getPitch() * 0.8f);
        if (world->multiplayerWorld) {
            return true;
        } else {
            world->setBlockWithNotify(i, j, k, block->blockID);
            itemstack->damageItem(1, entityplayer);
            return true;
        }
    } else {
        return false;
    }
}

bool ItemHoe::isFull3D() {
    return true;
}
