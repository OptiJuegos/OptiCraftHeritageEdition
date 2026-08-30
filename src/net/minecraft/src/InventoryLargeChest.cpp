#include "InventoryLargeChest.h"
#include "ItemStack.h"

InventoryLargeChest::InventoryLargeChest(std::string name, IInventory* upperChest, IInventory* lowerChest) {
    this->name = name;
    if (upperChest == nullptr) {
        upperChest = lowerChest;
    }
    if (lowerChest == nullptr) {
        lowerChest = upperChest;
    }
    this->upperChest = upperChest;
    this->lowerChest = lowerChest;
}

int_t InventoryLargeChest::getSizeInventory() {
    return upperChest->getSizeInventory() + lowerChest->getSizeInventory();
}

std::string InventoryLargeChest::getInvName() {
    return name;
}

ItemStack* InventoryLargeChest::getStackInSlot(int_t slot) {
    if (slot >= upperChest->getSizeInventory()) {
        return lowerChest->getStackInSlot(slot - upperChest->getSizeInventory());
    }
    return upperChest->getStackInSlot(slot);
}


ItemStack *InventoryLargeChest::getStackInSlotOnClosing(int_t i)
{
    if (i >= upperChest->getSizeInventory())
        return lowerChest->getStackInSlotOnClosing(i - upperChest->getSizeInventory());
    return upperChest->getStackInSlotOnClosing(i);
}

ItemStack* InventoryLargeChest::decrStackSize(int_t slot, int_t amount) {
    if (slot >= upperChest->getSizeInventory()) {
        return lowerChest->decrStackSize(slot - upperChest->getSizeInventory(), amount);
    }
    return upperChest->decrStackSize(slot, amount);
}

void InventoryLargeChest::setInventorySlotContents(int_t slot, ItemStack* stack) {
    if (slot >= upperChest->getSizeInventory()) {
        lowerChest->setInventorySlotContents(slot - upperChest->getSizeInventory(), stack);
    } else {
        upperChest->setInventorySlotContents(slot, stack);
    }
}

int_t InventoryLargeChest::getInventoryStackLimit() {
    return upperChest->getInventoryStackLimit();
}

void InventoryLargeChest::onInventoryChanged() {
    upperChest->onInventoryChanged();
    lowerChest->onInventoryChanged();
}

bool InventoryLargeChest::canInteractWith(EntityPlayer* player) {
    return upperChest->canInteractWith(player) && lowerChest->canInteractWith(player);
}
void InventoryLargeChest::openChest() {
    upperChest->openChest();
    lowerChest->openChest();
}

void InventoryLargeChest::closeChest() {
    upperChest->closeChest();
    lowerChest->closeChest();
}
