#pragma once

#include <string>
#include <vector>
#include "IInventory.h"
#include "IInvBasic.h"

class ItemStack;

class InventoryBasic : public IInventory {
public:
    InventoryBasic(std::string name, int slotsCount, bool ownsStacks = true);
    ~InventoryBasic() override;
    
    ItemStack* getStackInSlot(int_t slot) override;
    
    ItemStack *getStackInSlotOnClosing(int_t i) override;
    ItemStack* decrStackSize(int_t slot, int_t amount) override;
    void setInventorySlotContents(int_t slot, ItemStack* stack) override;
    int_t getSizeInventory() override;
    std::string getInvName() override;
    int_t getInventoryStackLimit() override;
    void onInventoryChanged() override;
    bool canInteractWith(EntityPlayer* player) override;
    
    void addInventoryChangeCallback(IInvBasic* callback);

private:
    std::string inventoryTitle;
    int slotsCount;
    ItemStack** inventoryContents;
    std::vector<IInvBasic*> changeCallbacks;
    bool ownsStacks;
};