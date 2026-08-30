#pragma once

#include "TileEntity.h"
#include "IInventory.h"
#include "java/Random.h"

class EntityPlayer;
class ItemStack;
class NBTTagCompound;

class TileEntityDispenser : public TileEntity, public IInventory {
public:
    TileEntityDispenser();
    ~TileEntityDispenser() override;

    int_t getSizeInventory() override;
    ItemStack* getStackInSlot(int_t i) override;
    ItemStack *getStackInSlotOnClosing(int_t i) override;
    ItemStack* decrStackSize(int_t i, int_t j) override;
    void setInventorySlotContents(int_t i, ItemStack* itemstack) override;
    std::string getInvName() override;
    int_t getInventoryStackLimit() override;

    // Bridge methods so the IInventory pure-virtual is satisfied by
    // delegating to TileEntity's non-virtual implementation.
    void onInventoryChanged() override { TileEntity::onInventoryChanged(); }
    bool canInteractWith(EntityPlayer* entityplayer) override;

    void readFromNBT(NBTTagCompound* nbttagcompound) override;
    void writeToNBT(NBTTagCompound* nbttagcompound) override;

    ItemStack* getRandomStackFromInventory();

private:
    ItemStack* dispenserContents[9];
    Random dispenserRandom;
};
