#pragma once

#include <string>
#include "IInventory.h"
#include "java/Type.h"

class ItemStack;

class InventoryLargeChest : public IInventory {
public:
    InventoryLargeChest(std::string name, IInventory* upperChest, IInventory* lowerChest);

    int_t getSizeInventory() override;
    std::string getInvName() override;
    ItemStack* getStackInSlot(int_t slot) override;
    ItemStack *getStackInSlotOnClosing(int_t i) override;
    ItemStack* decrStackSize(int_t slot, int_t amount) override;
    void setInventorySlotContents(int_t slot, ItemStack* stack) override;
    int_t getInventoryStackLimit() override;
    void onInventoryChanged() override;
    bool canInteractWith(EntityPlayer* player) override;
    void openChest() override;
    void closeChest() override;

private:
    std::string name;
    IInventory* upperChest;
    IInventory* lowerChest;
};