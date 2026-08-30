#include "ItemStack.h"
#include "Item.h"
#include "Block.h"
#include "EntityPlayer.h"
#include "EntityLiving.h"
#include "World.h"
#include "StatList.h"
#include <sstream>
#include "NBTTagCompound.h"
#include "NBTTagList.h"
#include "Enchantment.h"
#include "EnchantmentHelper.h"
#include "java/Arithmetic.h"

namespace
{
bool areTagCompoundsEqual(const NBTTagCompound* left, const NBTTagCompound* right)
{
    if (left == right) return true;
    if (left == nullptr || right == nullptr) return false;
    std::ostringstream leftStream(std::ios::binary);
    std::ostringstream rightStream(std::ios::binary);
    NBTBase::writeTag(const_cast<NBTTagCompound*>(left), leftStream);
    NBTBase::writeTag(const_cast<NBTTagCompound*>(right), rightStream);
    return leftStream.str() == rightStream.str();
}
}

ItemStack::ItemStack(Block* block) : ItemStack(block, 1) {}
ItemStack::ItemStack(Block* block, int i) : ItemStack(block->blockID, i, 0) {}
ItemStack::ItemStack(Block* block, int i, int j) : ItemStack(block->blockID, i, j) {}
ItemStack::ItemStack(Item* item) : ItemStack(item, 1) {}
ItemStack::ItemStack(Item* item, int i) : ItemStack(item->shiftedIndex, i, 0) {}
ItemStack::ItemStack(Item* item, int i, int j) : ItemStack(item->shiftedIndex, i, j) {}

ItemStack::ItemStack(int i, int j, int k) {
    stackSize = 0;
    itemID = i;
    stackSize = j;
    itemDamage = k;
    animationsToGo = 0;
}

ItemStack::ItemStack(NBTTagCompound* nbttagcompound) {
    stackSize = 0;
    itemID = 0;
    itemDamage = 0;
    animationsToGo = 0;
    readFromNBT(nbttagcompound);
}

ItemStack::~ItemStack() {
    delete stackTagCompound;
    stackTagCompound = nullptr;
}

ItemStack* ItemStack::loadItemStackFromNBT(NBTTagCompound* nbt) {
    if (nbt == nullptr) {
        return nullptr;
    }

    ItemStack* stack = new ItemStack(nbt);
    if (stack->getItem() == nullptr) {
        delete stack;
        return nullptr;
    }
    return stack;
}


bool ItemStack::isValidItemID(int id) {
    return id >= 0 && id < Item::ITEM_LIST_SIZE && Item::itemsList[id] != nullptr;
}

bool ItemStack::isValid() const {
    return stackSize > 0 && isValidItemID(itemID);
}

ItemStack* ItemStack::splitStack(int i) {
    ItemStack* result = new ItemStack(itemID, i, itemDamage);
    if (stackTagCompound != nullptr)
        result->stackTagCompound = static_cast<NBTTagCompound*>(stackTagCompound->copy());
    stackSize -= i;
    return result;
}

Item* ItemStack::getItem() {
    return isValidItemID(itemID) ? Item::itemsList[itemID] : nullptr;
}

int ItemStack::getIconIndex() {
    Item* item = getItem();
    return item ? item->getIconIndex(this) : 0;
}

bool ItemStack::useItem(EntityPlayer* entityplayer, World* world, int i, int j, int k, int l) {
    Item* item = getItem();
    if (!item) return false;
    bool flag = item->onItemUse(this, entityplayer, world, i, j, k, l);
    if (flag && itemID >= 0) {
        entityplayer->addStat(StatList::useItemStatArray[itemID], 1);
    }
    return flag;
}

float ItemStack::getStrVsBlock(Block* block) {
    Item* item = getItem();
    return item ? item->getStrVsBlock(this, block) : 1.0f;
}

ItemStack* ItemStack::useItemRightClick(World* world, EntityPlayer* entityplayer) {
    Item* item = getItem();
    return item ? item->onItemRightClick(this, world, entityplayer) : this;
}

ItemStack* ItemStack::onFoodEaten(World* world, EntityPlayer* entityplayer) {
    Item* item = getItem();
    return item ? item->onFoodEaten(this, world, entityplayer) : this;
}

int ItemStack::getMaxItemUseDuration() {
    Item* item = getItem();
    return item ? item->getMaxItemUseDuration(this) : 0;
}

EnumAction ItemStack::getItemUseAction() {
    Item* item = getItem();
    return item ? item->getItemUseAction(this) : EnumAction::none;
}

void ItemStack::onPlayerStoppedUsing(World* world, EntityPlayer* entityplayer, int timeLeft) {
    Item* item = getItem();
    if (item != nullptr)
        item->onPlayerStoppedUsing(this, world, entityplayer, timeLeft);
}

NBTTagCompound* ItemStack::writeToNBT(NBTTagCompound* nbttagcompound) {
    nbttagcompound->setShort("id", (short)itemID);
    nbttagcompound->setByte("Count", (char)stackSize);
    nbttagcompound->setShort("Damage", (short)itemDamage);
    if (stackTagCompound != nullptr)
        nbttagcompound->setTag("tag", stackTagCompound->copy());
    return nbttagcompound;
}

void ItemStack::readFromNBT(NBTTagCompound* nbttagcompound) {
    itemID = nbttagcompound->getShort("id");
    stackSize = nbttagcompound->getByte("Count");
    itemDamage = nbttagcompound->getShort("Damage");
    delete stackTagCompound;
    stackTagCompound = nullptr;
    if (nbttagcompound->hasKey("tag")) {
        NBTTagCompound* tag = nbttagcompound->getCompoundTag("tag");
        stackTagCompound = static_cast<NBTTagCompound*>(tag->copy());
    }
}

int ItemStack::getMaxStackSize() {
    Item* item = getItem();
    return item ? item->getItemStackLimit() : 64;
}

bool ItemStack::isStackable() {
    return getMaxStackSize() > 1 && (!isItemStackDamageable() || !isItemDamaged());
}

bool ItemStack::isItemStackDamageable() {
    Item* item = getItem();
    return item && item->getMaxDamage() > 0;
}

bool ItemStack::getHasSubtypes() {
    Item* item = getItem();
    return item && item->getHasSubtypes();
}

bool ItemStack::isItemDamaged() {
    return isItemStackDamageable() && itemDamage > 0;
}

int ItemStack::getItemDamageForDisplay() {
    return itemDamage;
}

int ItemStack::getItemDamage() {
    return itemDamage;
}

void ItemStack::setItemDamage(int i) {
    itemDamage = i;
}

int ItemStack::getMaxDamage() {
    Item* item = getItem();
    return item ? item->getMaxDamage() : 0;
}

void ItemStack::damageItem(int i, Entity* entity) {
    if (!isItemStackDamageable()) {
        return;
    }
    EntityPlayer* player = dynamic_cast<EntityPlayer*>(entity);
    if (i > 0 && player != nullptr) {
        const int unbreaking = EnchantmentHelper::getUnbreakingModifier(player->inventory);
        if (unbreaking > 0 && entity->worldObj != nullptr && entity->worldObj->rand.nextInt(unbreaking + 1) > 0)
            return;
    }
    itemDamage += i;
    if (itemDamage > getMaxDamage()) {
        EntityLiving* living = dynamic_cast<EntityLiving*>(entity);
        if (living != nullptr)
            living->renderBrokenItemStack(this);
        if (player != nullptr)
            player->addStat(StatList::breakItemStatArray[itemID], 1);
        stackSize--;
        if (stackSize < 0) stackSize = 0;
        itemDamage = 0;
    }
}

void ItemStack::hitEntity(EntityLiving* entityliving, EntityPlayer* entityplayer) {
    Item* item = getItem();
    if (!item) return;
    bool flag = item->hitEntity(this, entityliving, entityplayer);
    if (flag) {
        entityplayer->addStat(StatList::useItemStatArray[itemID], 1);
    }
}

void ItemStack::onDestroyBlock(int i, int j, int k, int l, EntityPlayer* entityplayer) {
    Item* item = getItem();
    if (!item) return;
    bool flag = item->onBlockDestroyed(this, i, j, k, l, entityplayer);
    if (flag) {
        entityplayer->addStat(StatList::useItemStatArray[itemID], 1);
    }
}

int ItemStack::getDamageVsEntity(Entity* entity) {
    Item* item = getItem();
    return item ? item->getDamageVsEntity(entity) : 1;
}

bool ItemStack::canHarvestBlock(Block* block) {
    Item* item = getItem();
    return item && item->canHarvestBlock(block);
}

void ItemStack::onItemDestroyedByUse(EntityPlayer*) {
}

void ItemStack::onItemUse(EntityPlayer* entityplayer) {
}

void ItemStack::useItemOnEntity(EntityLiving* entityliving) {
    Item* item = getItem();
    if (item) item->useItemOnEntity(this, entityliving);
}

ItemStack* ItemStack::copy() {
    ItemStack* result = new ItemStack(itemID, stackSize, itemDamage);
    if (stackTagCompound != nullptr)
        result->stackTagCompound = static_cast<NBTTagCompound*>(stackTagCompound->copy());
    return result;
}

bool ItemStack::areItemStackTagsEqual(ItemStack* itemstack, ItemStack* itemstack1) {
    if (itemstack == nullptr && itemstack1 == nullptr) return true;
    if (itemstack == nullptr || itemstack1 == nullptr) return false;
    return areTagCompoundsEqual(itemstack->stackTagCompound, itemstack1->stackTagCompound);
}

bool ItemStack::func_46154_a(ItemStack* itemstack, ItemStack* itemstack1) {
    return areItemStackTagsEqual(itemstack, itemstack1);
}

bool ItemStack::areItemStacksEqual(ItemStack* itemstack, ItemStack* itemstack1) {
    if (itemstack == nullptr && itemstack1 == nullptr) {
        return true;
    }
    if (itemstack == nullptr || itemstack1 == nullptr) {
        return false;
    }
    return itemstack->isItemStackEqual(itemstack1);
}

bool ItemStack::isItemStackEqual(ItemStack* itemstack) {
    if (stackSize != itemstack->stackSize) return false;
    if (itemID != itemstack->itemID) return false;
    if (itemDamage != itemstack->itemDamage) return false;
    return areTagCompoundsEqual(stackTagCompound, itemstack->stackTagCompound);
}

bool ItemStack::isItemEqual(ItemStack* itemstack) {
    return itemID == itemstack->itemID && itemDamage == itemstack->itemDamage;
}

std::string ItemStack::getItemName() {
    Item* item = getItem();
    return item ? item->getItemNameIS(this) : "item.invalid";
}

std::vector<std::string> ItemStack::getItemNameandInformation()
{
    std::vector<std::string> information;
    Item *item = getItem();
    if (item == nullptr)
    {
        information.push_back("item.invalid");
        return information;
    }

    information.push_back(item->getItemDisplayName(this));
    item->addInformation(this, information);

    NBTTagList *enchantments = getEnchantmentTagList();
    if (enchantments != nullptr)
    {
        Enchantment::initEnchantments();
        for (int_t i = 0; i < enchantments->tagCount(); ++i)
        {
            NBTTagCompound *tag = dynamic_cast<NBTTagCompound *>(enchantments->tagAt(i));
            if (tag == nullptr)
                continue;

            int_t enchantmentId = tag->getShort("id");
            int_t level = tag->getShort("lvl");
            if (enchantmentId >= 0 && enchantmentId < 256 && Enchantment::enchantmentsList[enchantmentId] != nullptr)
                information.push_back(Enchantment::enchantmentsList[enchantmentId]->getTranslatedName(level));
        }
    }

    return information;
}

ItemStack* ItemStack::copyItemStack(ItemStack* itemstack) {
    return itemstack != nullptr ? itemstack->copy() : nullptr;
}

std::string ItemStack::toString() {
    std::ostringstream oss;
    Item* item = getItem();
    oss << stackSize << "x" << (item ? item->getItemName() : "item.invalid") << "@" << itemDamage;
    return oss.str();
}

void ItemStack::updateAnimation(World* world, Entity* entity, int i, bool flag) {
    if (animationsToGo > 0) {
        animationsToGo--;
    }
    Item* item = getItem();
    if (item) item->onUpdate(this, world, entity, i, flag);
}

void ItemStack::onCrafting(World* world, EntityPlayer* entityplayer) {
    onCrafting(world, entityplayer, stackSize);
}

void ItemStack::onCrafting(World* world, EntityPlayer* entityplayer, int_t amount) {
    Item* item = getItem();
    if (!item || entityplayer == nullptr) return;
    entityplayer->addStat(StatList::craftItemStatArray[itemID], amount);
    item->onCreated(this, world, entityplayer);
}

bool ItemStack::isStackEqual(ItemStack* itemstack) {
    return itemID == itemstack->itemID && stackSize == itemstack->stackSize && itemDamage == itemstack->itemDamage;
}

bool ItemStack::hasEffect()
{
    Item *item = getItem();
    return item != nullptr && item->hasEffect(this);
}

const EnumRarity& ItemStack::getRarity()
{
    Item *item = getItem();
    return item != nullptr ? item->getRarity(this) : EnumRarity::common;
}


bool ItemStack::hasTagCompound() const {
    return stackTagCompound != nullptr;
}

NBTTagCompound* ItemStack::getTagCompound() const {
    return stackTagCompound;
}

NBTTagList* ItemStack::getEnchantmentTagList() const {
    if (stackTagCompound == nullptr || !stackTagCompound->hasKey("ench"))
        return nullptr;
    return stackTagCompound->getTagList("ench");
}

void ItemStack::setTagCompound(NBTTagCompound* nbt) {
    if (stackTagCompound == nbt) return;
    delete stackTagCompound;
    stackTagCompound = nbt;
}

bool ItemStack::isItemEnchantable() {
    Item* item = getItem();
    return item != nullptr && item->isItemTool(this) && !isItemEnchanted();
}

void ItemStack::addEnchantment(Enchantment* enchantment, int level) {
    if (enchantment == nullptr) return;
    if (stackTagCompound == nullptr)
        stackTagCompound = new NBTTagCompound();
    if (!stackTagCompound->hasKey("ench"))
        stackTagCompound->setTag("ench", new NBTTagList());
    NBTTagList* enchantments = stackTagCompound->getTagList("ench");
    NBTTagCompound* entry = new NBTTagCompound();
    entry->setShort("id", static_cast<short_t>(enchantment->effectId));
    entry->setShort("lvl", static_cast<short_t>(JavaArithmetic::byteFromBits(static_cast<ubyte_t>(level))));
    enchantments->appendTag(entry);
}

bool ItemStack::isItemEnchanted() const {
    return stackTagCompound != nullptr && stackTagCompound->hasKey("ench");
}
