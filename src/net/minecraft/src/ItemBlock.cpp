#include "ItemBlock.h"
#include "ItemStack.h"
#include "Block.h"
#include "BlockDeadBush.h"
#include "BlockTallGrass.h"
#include "BlockVine.h"
#include "World.h"
#include "Material.h"
#include "StepSound.h"
#include "EntityPlayer.h"

ItemBlock::ItemBlock(int i)
    : Item(i) {
    blockID = i + 256;
    setIconIndex(Block::blocksList[i + 256]->getBlockTextureFromSide(2));
}

int ItemBlock::getBlockID() const {
    return blockID;
}

bool ItemBlock::onItemUse(ItemStack* itemstack, EntityPlayer* entityplayer, World* world, int i, int j, int k, int l) {
    int targetId = world->getBlockId(i, j, k);
    if (Block::snow != nullptr && targetId == Block::snow->blockID) {
        l = 1;
    } else if ((Block::vine == nullptr || targetId != Block::vine->blockID) &&
               (Block::tallGrass == nullptr || targetId != Block::tallGrass->blockID) &&
               (Block::deadBush == nullptr || targetId != Block::deadBush->blockID)) {
        if (l == 0) j--;
        if (l == 1) j++;
        if (l == 2) k--;
        if (l == 3) k++;
        if (l == 4) i--;
        if (l == 5) i++;
    }
    if (itemstack->stackSize == 0) {
        return false;
    }
    if (entityplayer != nullptr && !entityplayer->canPlayerEdit(i, j, k)) {
        return false;
    }
    Block* placedBlock = blockID >= 0 && blockID < Block::BLOCK_REGISTRY_SIZE ? Block::blocksList[blockID] : nullptr;
    if (placedBlock == nullptr) {
        return false;
    }
    if (j == 255 && placedBlock->blockMaterial->isSolid()) {
        return false;
    }
    if (world->canBlockBePlacedAt(blockID, i, j, k, false, l)) {
        if (world->setBlockAndMetadataWithNotify(i, j, k, blockID, getMetadata(itemstack->getItemDamage()))) {
            if (world->getBlockId(i, j, k) == blockID) {
                placedBlock->onBlockPlaced(world, i, j, k, l);
                placedBlock->onBlockPlacedBy(world, i, j, k, entityplayer);
            }
            world->playSoundEffect((float)i + 0.5f, (float)j + 0.5f, (float)k + 0.5f, placedBlock->stepSound->getStepSound(), (placedBlock->stepSound->getVolume() + 1.0f) / 2.0f, placedBlock->stepSound->getPitch() * 0.8f);
            itemstack->stackSize--;
        }
        return true;
    }
    return false;
}

std::string ItemBlock::getItemNameIS(ItemStack* itemstack) {
    return Block::blocksList[blockID]->getBlockName();
}

std::string ItemBlock::getItemName() {
    return Block::blocksList[blockID]->getBlockName();
}
