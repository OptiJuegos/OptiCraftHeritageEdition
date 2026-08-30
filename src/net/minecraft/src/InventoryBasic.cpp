#include "InventoryBasic.h"
#include "ItemStack.h"

InventoryBasic::InventoryBasic(std::string name, int slotsCount, bool ownsStacksIn) {
    this->inventoryTitle = name;
    this->slotsCount = slotsCount;
    this->ownsStacks = ownsStacksIn;
    this->inventoryContents = new ItemStack*[slotsCount]();
    for (int i = 0; i < slotsCount; i++) {
        this->inventoryContents[i] = nullptr;
    }
}

InventoryBasic::~InventoryBasic() {
    if (inventoryContents != nullptr) {
        if (ownsStacks) {
            for (int i = 0; i < slotsCount; ++i) {
                delete inventoryContents[i];
                inventoryContents[i] = nullptr;
            }
        }
        delete[] inventoryContents;
        inventoryContents = nullptr;
    }
}

ItemStack* InventoryBasic::getStackInSlot(int_t slot) {
    return slot >= 0 && slot < slotsCount ? inventoryContents[slot] : nullptr;
}


ItemStack *InventoryBasic::getStackInSlotOnClosing(int_t i)
{
    if (i < 0 || i >= slotsCount || inventoryContents[i] == nullptr)
        return nullptr;
    ItemStack *stack = inventoryContents[i];
    inventoryContents[i] = nullptr;
    return stack;
}

ItemStack* InventoryBasic::decrStackSize(int_t slot, int_t amount) {
    if (slot < 0 || slot >= slotsCount || amount <= 0)
        return nullptr;
    if (inventoryContents[slot] != nullptr) {
        if (inventoryContents[slot]->stackSize <= amount) {
            ItemStack* stack = inventoryContents[slot];
            inventoryContents[slot] = nullptr;
            onInventoryChanged();
            return stack;
        }
        ItemStack* split = inventoryContents[slot]->splitStack(amount);
        if (inventoryContents[slot]->stackSize == 0) {
            if (ownsStacks)
                delete inventoryContents[slot];
            inventoryContents[slot] = nullptr;
        }
        onInventoryChanged();
        return split;
    }
    return nullptr;
}

void InventoryBasic::setInventorySlotContents(int_t slot, ItemStack* stack) {
    if (slot < 0 || slot >= slotsCount) {
        if (ownsStacks)
            delete stack;
        return;
    }
    if (ownsStacks && inventoryContents[slot] != stack)
        delete inventoryContents[slot];
    inventoryContents[slot] = stack;
    if (stack != nullptr && stack->stackSize > getInventoryStackLimit()) {
        stack->stackSize = getInventoryStackLimit();
    }
    onInventoryChanged();
}

int_t InventoryBasic::getSizeInventory() {
    return slotsCount;
}

std::string InventoryBasic::getInvName() {
    return inventoryTitle;
}

int_t InventoryBasic::getInventoryStackLimit() {
    return 64;
}

void InventoryBasic::onInventoryChanged() {
    for (size_t i = 0; i < changeCallbacks.size(); i++) {
        changeCallbacks[i]->onInventoryChanged(this);
    }
}

bool InventoryBasic::canInteractWith(EntityPlayer* player) {
    return true;
}

void InventoryBasic::addInventoryChangeCallback(IInvBasic* callback) {
    changeCallbacks.push_back(callback);
}