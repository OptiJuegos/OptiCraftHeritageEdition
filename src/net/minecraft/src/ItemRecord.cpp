#include "ItemRecord.h"
#include "ItemStack.h"
#include "World.h"
#include "Block.h"
#include "BlockJukeBox.h"
#include "EntityPlayer.h"

ItemRecord::ItemRecord(int i, const std::string& s)
    : Item(i), recordName(s) {
    maxStackSize = 1;
}

bool ItemRecord::onItemUse(ItemStack* itemstack, EntityPlayer* entityplayer, World* world, int i, int j, int k, int l) {
    if (world->getBlockId(i, j, k) == Block::jukebox->blockID && world->getBlockMetadata(i, j, k) == 0) {
        if (world->multiplayerWorld) {
            return true;
        } else {
            ((BlockJukeBox*)Block::jukebox)->insertRecord(world, i, j, k, shiftedIndex);
            world->playAuxSFXAtEntity(nullptr, 1005, i, j, k, shiftedIndex);
            itemstack->stackSize--;
            return true;
        }
    } else {
        return false;
    }
}

void ItemRecord::addInformation(ItemStack *, std::vector<std::string>& information)
{
    information.push_back("C418 - " + recordName);
}

const EnumRarity& ItemRecord::getRarity(ItemStack *)
{
    return EnumRarity::rare;
}
