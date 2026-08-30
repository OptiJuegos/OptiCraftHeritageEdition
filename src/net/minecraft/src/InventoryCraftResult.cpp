#include "InventoryCraftResult.h"
#include "ItemStack.h"

InventoryCraftResult::InventoryCraftResult() {
    stackResult = new ItemStack*[1]();
    stackResult[0] = nullptr;
}

InventoryCraftResult::~InventoryCraftResult() {
    delete stackResult[0];
    delete[] stackResult;
}

int_t InventoryCraftResult::getSizeInventory() {
    return 1;
}

ItemStack* InventoryCraftResult::getStackInSlot(int_t slot) {
    return slot == 0 ? stackResult[0] : nullptr;
}

std::string InventoryCraftResult::getInvName() {
    return "Result";
}


ItemStack *InventoryCraftResult::getStackInSlotOnClosing(int_t i)
{
    if (i != 0 || stackResult[0] == nullptr)
        return nullptr;
    ItemStack *stack = stackResult[i];
    stackResult[i] = nullptr;
    return stack;
}

ItemStack* InventoryCraftResult::decrStackSize(int_t slot, int_t amount) {
    if (slot == 0 && amount > 0 && stackResult[0] != nullptr) {
        ItemStack* stack = stackResult[slot];
        stackResult[slot] = nullptr;
        return stack;
    }
    return nullptr;
}

void InventoryCraftResult::setInventorySlotContents(int_t slot, ItemStack* stack) {
    if (slot != 0) {
        delete stack;
        return;
    }
    if (stackResult[0] != stack)
        delete stackResult[slot];
    stackResult[slot] = stack;
}

int_t InventoryCraftResult::getInventoryStackLimit() {
    return 64;
}

void InventoryCraftResult::onInventoryChanged() {
}

bool InventoryCraftResult::canInteractWith(EntityPlayer* player) {
    return true;
}
