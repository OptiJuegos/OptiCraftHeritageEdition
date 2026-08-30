#pragma once

#include "IInventory.h"
#include "TileEntity.h"

class ItemStack;
class NBTTagCompound;
class EntityPlayer;

// net.minecraft.src.TileEntityBrewingStand
class TileEntityBrewingStand : public TileEntity, public IInventory
{
public:
    TileEntityBrewingStand();
    ~TileEntityBrewingStand() override;

    std::string getInvName() override;
    int_t getSizeInventory() override;
    void updateEntity() override;
    int_t getBrewTime() const;
    ItemStack *getStackInSlot(int_t slot) override;
    ItemStack *decrStackSize(int_t slot, int_t amount) override;
    ItemStack *getStackInSlotOnClosing(int_t slot) override;
    void setInventorySlotContents(int_t slot, ItemStack *itemstack) override;
    int_t getInventoryStackLimit() override;
    bool canInteractWith(EntityPlayer *player) override;
    void onInventoryChanged() override { TileEntity::onInventoryChanged(); }
    void readFromNBT(NBTTagCompound *compound) override;
    void writeToNBT(NBTTagCompound *compound) override;
    void setBrewTime(int_t time);
    int_t getFilledSlots() const;

private:
    bool canBrew() const;
    void brewPotions();
    int_t getPotionResult(int_t damage, ItemStack *ingredient) const;

    ItemStack *brewingItemStacks[4];
    int_t brewTime = 0;
    int_t filledSlots = 0;
    int_t ingredientID = -1;
};
