#pragma once

#include "IInventory.h"

class ItemStack;

class InventoryCraftResult : public IInventory {
public:
    InventoryCraftResult();
    ~InventoryCraftResult() override;
    
    int_t getSizeInventory() override;
    ItemStack* getStackInSlot(int_t slot) override;
    ItemStack *getStackInSlotOnClosing(int_t i) override;
    std::string getInvName() override;
    ItemStack* decrStackSize(int_t slot, int_t amount) override;
    void setInventorySlotContents(int_t slot, ItemStack* stack) override;
    int_t getInventoryStackLimit() override;
    void onInventoryChanged() override;
    bool canInteractWith(EntityPlayer* player) override;

private:
    ItemStack** stackResult;
};
