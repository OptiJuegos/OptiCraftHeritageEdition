#pragma once

#include <string>

#include "IInventory.h"
#include "TileEntity.h"
#include "java/Type.h"

class EntityPlayer;
class ItemStack;
class NBTTagCompound;

// net.minecraft.src.TileEntityFurnace
class TileEntityFurnace : public TileEntity, public IInventory
{
public:
    TileEntityFurnace();
    ~TileEntityFurnace() override;

    int_t getSizeInventory() override;
    ItemStack *getStackInSlot(int_t i) override;
    ItemStack *getStackInSlotOnClosing(int_t i) override;
    ItemStack *decrStackSize(int_t i, int_t j) override;
    void setInventorySlotContents(int_t i, ItemStack *itemstack) override;
    std::string getInvName() override;
    int_t getInventoryStackLimit() override;
    void onInventoryChanged() override { TileEntity::onInventoryChanged(); }
    bool canInteractWith(EntityPlayer *entityplayer) override;
    void readFromNBT(NBTTagCompound *nbttagcompound) override;
    void writeToNBT(NBTTagCompound *nbttagcompound) override;
    int_t getCookProgressScaled(int_t i);
    int_t getBurnTimeRemainingScaled(int_t i);
    bool isBurning();
    void updateEntity() override;
    bool canSmelt();
    void smeltItem();
    static int_t getItemBurnTime(ItemStack *itemstack);
    static bool isItemFuel(ItemStack *itemstack);

public:
    ItemStack *furnaceItemStacks[3];
    int_t furnaceBurnTime;
    int_t currentItemBurnTime;
    int_t furnaceCookTime;
};
