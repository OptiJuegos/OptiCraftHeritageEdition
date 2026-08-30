#include "TileEntityDispenser.h"

#include "NBTTagCompound.h"
#include "NBTTagList.h"
#include "ItemStack.h"
#include "EntityPlayer.h"
#include "World.h"

TileEntityDispenser::TileEntityDispenser() {
    for (int_t i = 0; i < 9; i++) dispenserContents[i] = nullptr;
}

TileEntityDispenser::~TileEntityDispenser() {
    for (int_t i = 0; i < 9; ++i) {
        delete dispenserContents[i];
        dispenserContents[i] = nullptr;
    }
}

int_t TileEntityDispenser::getSizeInventory() {
    return 9;
}

ItemStack* TileEntityDispenser::getStackInSlot(int_t i) {
    return i >= 0 && i < getSizeInventory() ? dispenserContents[i] : nullptr;
}


ItemStack *TileEntityDispenser::getStackInSlotOnClosing(int_t i)
{
    if (i < 0 || i >= getSizeInventory() || dispenserContents[i] == nullptr)
        return nullptr;
    ItemStack *stack = dispenserContents[i];
    dispenserContents[i] = nullptr;
    return stack;
}

ItemStack* TileEntityDispenser::decrStackSize(int_t i, int_t j) {
    if (i < 0 || i >= getSizeInventory() || j <= 0)
        return nullptr;
    if (dispenserContents[i] != nullptr) {
        if (dispenserContents[i]->stackSize <= j) {
            ItemStack* itemstack = dispenserContents[i];
            dispenserContents[i] = nullptr;
            onInventoryChanged();
            return itemstack;
        }
        ItemStack* itemstack1 = dispenserContents[i]->splitStack(j);
        if (dispenserContents[i]->stackSize == 0) {
            delete dispenserContents[i];
            dispenserContents[i] = nullptr;
        }
        onInventoryChanged();
        return itemstack1;
    } else {
        return nullptr;
    }
}

ItemStack* TileEntityDispenser::getRandomStackFromInventory() {
    int_t i = -1;
    int_t j = 1;
    for (int_t k = 0; k < (int_t)(sizeof(dispenserContents) / sizeof(dispenserContents[0])); k++) {
        if (dispenserContents[k] != nullptr && dispenserRandom.nextInt(j++) == 0) {
            i = k;
        }
    }
    if (i >= 0) {
        return decrStackSize(i, 1);
    } else {
        return nullptr;
    }
}

void TileEntityDispenser::setInventorySlotContents(int_t i, ItemStack* itemstack) {
    if (i < 0 || i >= getSizeInventory()) {
        delete itemstack;
        return;
    }
    if (dispenserContents[i] != itemstack)
        delete dispenserContents[i];
    dispenserContents[i] = itemstack;
    if (itemstack != nullptr && itemstack->stackSize > getInventoryStackLimit()) {
        itemstack->stackSize = getInventoryStackLimit();
    }
    onInventoryChanged();
}

std::string TileEntityDispenser::getInvName() {
    return "container.dispenser";
}

int_t TileEntityDispenser::getInventoryStackLimit() {
    return 64;
}

bool TileEntityDispenser::canInteractWith(EntityPlayer* entityplayer) {
    if (worldObj->getBlockTileEntity(xCoord, yCoord, zCoord) != this) {
        return false;
    }
    return entityplayer->getDistanceSq((double)xCoord + 0.5, (double)yCoord + 0.5, (double)zCoord + 0.5) <= 64.0;
}

void TileEntityDispenser::readFromNBT(NBTTagCompound* nbttagcompound) {
    TileEntity::readFromNBT(nbttagcompound);
    for (int_t i = 0; i < 9; ++i) {
        delete dispenserContents[i];
        dispenserContents[i] = nullptr;
    }
    NBTTagList* nbttaglist = nbttagcompound->getTagList("Items");
    for (int_t i = 0; i < 9; i++) dispenserContents[i] = nullptr;
    for (int_t i = 0; i < nbttaglist->tagCount(); i++) {
        NBTTagCompound* nbttagcompound1 = (NBTTagCompound*)nbttaglist->tagAt(i);
        int_t j = nbttagcompound1->getByte("Slot") & 0xff;
        if (j >= 0 && j < getSizeInventory()) {
            ItemStack *loaded = ItemStack::loadItemStackFromNBT(nbttagcompound1);
            delete dispenserContents[j];
            dispenserContents[j] = loaded;
        }
    }
}

void TileEntityDispenser::writeToNBT(NBTTagCompound* nbttagcompound) {
    TileEntity::writeToNBT(nbttagcompound);
    NBTTagList* nbttaglist = new NBTTagList();
    for (int_t i = 0; i < getSizeInventory(); i++) {
        if (dispenserContents[i] != nullptr) {
            NBTTagCompound* nbttagcompound1 = new NBTTagCompound();
            nbttagcompound1->setByte("Slot", (byte_t)i);
            dispenserContents[i]->writeToNBT(nbttagcompound1);
            nbttaglist->appendTag(nbttagcompound1);
        }
    }
    nbttagcompound->setTag("Items", nbttaglist);
}
