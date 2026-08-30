#pragma once

#include "IInventory.h"

class ItemStack;
class EntityPlayer;
class Block;
class Entity;
class NBTTagList;

class InventoryPlayer : public IInventory {
public:
    ItemStack** mainInventory;
    ItemStack** armorInventory;
    int currentItem;
    bool inventoryChanged;
    EntityPlayer* player;
    ItemStack* itemStack;

    InventoryPlayer(EntityPlayer* player);
    ~InventoryPlayer() override;
    
    ItemStack* getCurrentItem();
    int getInventorySlotContainItem(int itemID);
    int getInventorySlotContainItemAndDamage(int itemID, int damage);
    int storeItemStack(ItemStack* stack);
    int getFirstEmptyStack();
    void setCurrentItem(int itemID, bool createIfMissing);
    void setCurrentItem(int itemID, int damage, bool matchDamage, bool createIfMissing);
    void changeCurrentItem(int direction);
    int storePartialItemStack(ItemStack* stack);
    void decrementAnimations();
    bool consumeInventoryItem(int itemID);
    bool hasItem(int itemID);
    bool addItemStackToInventory(ItemStack* stack);
    float getStrVsBlock(Block* block);
    NBTTagList* writeToNBT(NBTTagList* tagList);
    void readFromNBT(NBTTagList* tagList);
    int getTotalArmorValue();
    void damageArmor(int amount);
    void dropAllItems();
    ItemStack* armorItemInSlot(int slot);
    int getDamageVsEntity(Entity* entity);
    bool canHarvestBlock(Block* block);
    void setItemStack(ItemStack* stack);
    ItemStack* getItemStack();
    bool isUseableByPlayer(EntityPlayer* player) const;
    bool hasItemStack(ItemStack* stack) const;
    void openChest();
    void closeChest();
    bool doesXZShowFog(ItemStack* stack);
    void copyInventory(const InventoryPlayer* other);
    
    // IInventory implementation
    int_t getSizeInventory() override;
    ItemStack* getStackInSlot(int_t slot) override;
    ItemStack *getStackInSlotOnClosing(int_t i) override;
    ItemStack* decrStackSize(int_t slot, int_t amount) override;
    void setInventorySlotContents(int_t slot, ItemStack* stack) override;
    std::string getInvName() override;
    int_t getInventoryStackLimit() override;
    void onInventoryChanged() override;
    bool canInteractWith(EntityPlayer* player) override;
};
