#pragma once

#include <string>
#include <vector>
#include "EnumAction.h"
#include "java/Type.h"

class Item;
class Block;
class NBTTagCompound;
class NBTTagList;
class Enchantment;
class World;
class EntityPlayer;
class EntityLiving;
class Entity;
class EnumRarity;

class ItemStack {
public:
    ItemStack(Block* block);
    ItemStack(Block* block, int count);
    ItemStack(Block* block, int count, int damage);
    ItemStack(Item* item);
    ItemStack(Item* item, int count);
    ItemStack(Item* item, int count, int damage);
    ItemStack(int id, int count, int damage);
    ItemStack(NBTTagCompound* nbt);
    ~ItemStack();

    static ItemStack* loadItemStackFromNBT(NBTTagCompound* nbt);

    ItemStack* splitStack(int amount);
    Item* getItem();
    int getIconIndex();
    bool useItem(EntityPlayer* entityplayer, World* world, int i, int j, int k, int l);
    float getStrVsBlock(Block* block);
    ItemStack* useItemRightClick(World* world, EntityPlayer* entityplayer);
    ItemStack* onFoodEaten(World* world, EntityPlayer* entityplayer);
    int getMaxItemUseDuration();
    EnumAction getItemUseAction();
    void onPlayerStoppedUsing(World* world, EntityPlayer* entityplayer, int timeLeft);
    NBTTagCompound* writeToNBT(NBTTagCompound* nbt);
    void readFromNBT(NBTTagCompound* nbt);
    int getMaxStackSize();
    bool isStackable();
    bool isItemStackDamageable();
    bool getHasSubtypes();
    bool isItemDamaged();
    int getItemDamageForDisplay();
    int getItemDamage();
    void setItemDamage(int damage);
    int getMaxDamage();
    void damageItem(int damage, Entity* entity);
    void hitEntity(EntityLiving* entityliving, EntityPlayer* entityplayer);
    void onDestroyBlock(int i, int j, int k, int l, EntityPlayer* entityplayer);
    int getDamageVsEntity(Entity* entity);
    bool canHarvestBlock(Block* block);
    void onItemDestroyedByUse(EntityPlayer* entityplayer);
    void onItemUse(EntityPlayer* entityplayer);
    void useItemOnEntity(EntityLiving* entityliving);
    ItemStack* copy();
    bool isValid() const;
    static bool isValidItemID(int id);
    static bool func_46154_a(ItemStack* a, ItemStack* b);
    static bool areItemStackTagsEqual(ItemStack* a, ItemStack* b);
    static bool areItemStacksEqual(ItemStack* a, ItemStack* b);
    bool isItemStackEqual(ItemStack* other);
    bool isItemEqual(ItemStack* other);
    std::string getItemName();
    std::vector<std::string> getItemNameandInformation();
    static ItemStack* copyItemStack(ItemStack* itemstack);
    std::string toString();
    void updateAnimation(World* world, Entity* entity, int i, bool flag);
    void onCrafting(World* world, EntityPlayer* entityplayer);
    void onCrafting(World* world, EntityPlayer* entityplayer, int_t amount);
    bool isStackEqual(ItemStack* other);
    bool hasEffect();
    const EnumRarity& getRarity();
    bool hasTagCompound() const;
    NBTTagCompound* getTagCompound() const;
    NBTTagList* getEnchantmentTagList() const;
    void setTagCompound(NBTTagCompound* nbt);
    bool isItemEnchantable();
    void addEnchantment(Enchantment* enchantment, int level);
    bool isItemEnchanted() const;

    int stackSize;
    // Java defaults this to 0. The ItemStack(NBTTagCompound*) ctor doesn't set it, so items LOADED
    // from a save had garbage here; a large positive value makes GuiIngame::renderInventorySlot scale
    // the icon to ~0 width (glScalef(1/f2,...)) → the hotbar icon vanishes while its count still shows.
    int animationsToGo = 0;
    int itemID;

private:
    int itemDamage;
    NBTTagCompound* stackTagCompound = nullptr;
};
