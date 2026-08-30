#include "InventoryPlayer.h"
#include "ItemStack.h"
#include "Item.h"
#include "EntityPlayer.h"
#include "Block.h"
#include "Entity.h"
#include "ItemArmor.h"
#include "NBTTagList.h"
#include "NBTTagCompound.h"
#include "Material.h"

InventoryPlayer::InventoryPlayer(EntityPlayer* player) {
    mainInventory = new ItemStack*[36]();
    armorInventory = new ItemStack*[4]();
    for (int i = 0; i < 36; i++) {
        mainInventory[i] = nullptr;
    }
    for (int i = 0; i < 4; i++) {
        armorInventory[i] = nullptr;
    }
    currentItem = 0;
    inventoryChanged = false;
    this->player = player;
    itemStack = nullptr;
}

InventoryPlayer::~InventoryPlayer()
{
    for (int i = 0; i < 36; ++i)
        delete mainInventory[i];
    for (int i = 0; i < 4; ++i)
        delete armorInventory[i];
    delete itemStack;
    delete[] mainInventory;
    delete[] armorInventory;
}

ItemStack* InventoryPlayer::getCurrentItem() {
    if (currentItem < 9 && currentItem >= 0) {
        ItemStack* stack = mainInventory[currentItem];
        return (stack != nullptr && stack->isValid()) ? stack : nullptr;
    }
    return nullptr;
}

int InventoryPlayer::getInventorySlotContainItem(int itemID) {
    for (int i = 0; i < 36; i++) {
        if (mainInventory[i] != nullptr && mainInventory[i]->itemID == itemID) {
            return i;
        }
    }
    return -1;
}

int InventoryPlayer::getInventorySlotContainItemAndDamage(int itemID, int damage) {
    for (int i = 0; i < 36; ++i) {
        if (mainInventory[i] != nullptr && mainInventory[i]->itemID == itemID &&
            mainInventory[i]->getItemDamage() == damage) {
            return i;
        }
    }
    return -1;
}

int InventoryPlayer::storeItemStack(ItemStack* stack) {
    if (stack == nullptr || !stack->isValid()) {
        return -1;
    }
    for (int i = 0; i < 36; i++) {
        if (mainInventory[i] != nullptr && 
            mainInventory[i]->itemID == stack->itemID && 
            mainInventory[i]->isStackable() && 
            mainInventory[i]->stackSize < mainInventory[i]->getMaxStackSize() && 
            mainInventory[i]->stackSize < getInventoryStackLimit() && 
            (!mainInventory[i]->getHasSubtypes() || mainInventory[i]->getItemDamage() == stack->getItemDamage()) &&
            ItemStack::func_46154_a(mainInventory[i], stack)) {
            return i;
        }
    }
    return -1;
}

int InventoryPlayer::getFirstEmptyStack() {
    for (int i = 0; i < 36; i++) {
        if (mainInventory[i] == nullptr) {
            return i;
        }
    }
    return -1;
}

void InventoryPlayer::setCurrentItem(int itemID, bool createIfMissing) {
    setCurrentItem(itemID, 0, false, createIfMissing);
}

void InventoryPlayer::setCurrentItem(int itemID, int damage, bool matchDamage, bool createIfMissing) {
    const int slot = matchDamage ? getInventorySlotContainItemAndDamage(itemID, damage)
                                 : getInventorySlotContainItem(itemID);
    if (slot >= 0 && slot < 9) {
        currentItem = slot;
        return;
    }

    if (!createIfMissing || itemID <= 0 || itemID >= Item::ITEM_LIST_SIZE || Item::itemsList[itemID] == nullptr)
        return;

    int emptySlot = getFirstEmptyStack();
    if (emptySlot >= 0 && emptySlot < 9)
        currentItem = emptySlot;

    const int matchingSlot = getInventorySlotContainItemAndDamage(itemID, damage);
    if (matchingSlot >= 0) {
        if (matchingSlot != currentItem) {
            delete mainInventory[matchingSlot];
            mainInventory[matchingSlot] = mainInventory[currentItem];
            mainInventory[currentItem] = nullptr;
        } else {
            delete mainInventory[currentItem];
            mainInventory[currentItem] = nullptr;
        }
    }

    delete mainInventory[currentItem];
    mainInventory[currentItem] = new ItemStack(Item::itemsList[itemID], 1, damage);
}

void InventoryPlayer::changeCurrentItem(int direction) {
    if (direction > 0) {
        direction = 1;
    }
    if (direction < 0) {
        direction = -1;
    }
    currentItem -= direction;
    while (currentItem < 0) {
        currentItem += 9;
    }
    while (currentItem >= 9) {
        currentItem -= 9;
    }
}

int InventoryPlayer::storePartialItemStack(ItemStack* stack) {
    if (stack == nullptr || !stack->isValid()) {
        return stack != nullptr ? stack->stackSize : 0;
    }

    const int itemID = stack->itemID;
    int count = stack->stackSize;

    if (stack->getMaxStackSize() == 1) {
        const int slot = getFirstEmptyStack();
        if (slot < 0) {
            return count;
        }
        if (mainInventory[slot] == nullptr) {
            mainInventory[slot] = ItemStack::copyItemStack(stack);
        }
        return 0;
    }

    int slot = storeItemStack(stack);
    if (slot < 0) {
        slot = getFirstEmptyStack();
    }
    if (slot < 0) {
        return count;
    }

    if (mainInventory[slot] == nullptr) {
        mainInventory[slot] = new ItemStack(itemID, 0, stack->getItemDamage());
        if (stack->hasTagCompound()) {
            NBTTagCompound *tagCopy = dynamic_cast<NBTTagCompound *>(stack->getTagCompound()->copy());
            mainInventory[slot]->setTagCompound(tagCopy);
        }
    }

    int transfer = count;
    if (transfer > mainInventory[slot]->getMaxStackSize() - mainInventory[slot]->stackSize) {
        transfer = mainInventory[slot]->getMaxStackSize() - mainInventory[slot]->stackSize;
    }
    if (transfer > getInventoryStackLimit() - mainInventory[slot]->stackSize) {
        transfer = getInventoryStackLimit() - mainInventory[slot]->stackSize;
    }
    if (transfer == 0) {
        return count;
    }

    count -= transfer;
    mainInventory[slot]->stackSize += transfer;
    mainInventory[slot]->animationsToGo = 5;
    return count;
}

void InventoryPlayer::decrementAnimations() {
    for (int i = 0; i < 36; i++) {
        if (mainInventory[i] != nullptr && mainInventory[i]->isValid()) {
            mainInventory[i]->updateAnimation(player->worldObj, player, i, currentItem == i);
        }
    }
}

bool InventoryPlayer::consumeInventoryItem(int itemID) {
    int slot = getInventorySlotContainItem(itemID);
    if (slot < 0) {
        return false;
    }
    if (--mainInventory[slot]->stackSize <= 0) {
        delete mainInventory[slot];
        mainInventory[slot] = nullptr;
    }
    return true;
}

bool InventoryPlayer::hasItem(int itemID) {
    return getInventorySlotContainItem(itemID) >= 0;
}

bool InventoryPlayer::addItemStackToInventory(ItemStack* stack) {
    if (stack == nullptr || !stack->isValid()) {
        return false;
    }

    if (stack->isItemDamaged()) {
        const int slot = getFirstEmptyStack();
        if (slot >= 0) {
            mainInventory[slot] = ItemStack::copyItemStack(stack);
            mainInventory[slot]->animationsToGo = 5;
            stack->stackSize = 0;
            return true;
        }
        if (player->capabilities.isCreativeMode) {
            stack->stackSize = 0;
            return true;
        }
        return false;
    }

    int previousCount;
    do {
        previousCount = stack->stackSize;
        stack->stackSize = storePartialItemStack(stack);
    } while (stack->stackSize > 0 && stack->stackSize < previousCount);

    if (stack->stackSize == previousCount && player->capabilities.isCreativeMode) {
        stack->stackSize = 0;
        return true;
    }
    return stack->stackSize < previousCount;
}

int_t InventoryPlayer::getSizeInventory() {
    return 36 + 4;
}

ItemStack* InventoryPlayer::getStackInSlot(int_t slot) {
    if (slot < 0 || slot >= getSizeInventory()) {
        return nullptr;
    }
    ItemStack** inventory = mainInventory;
    if (slot >= 36) {
        slot -= 36;
        inventory = armorInventory;
    }
    return inventory[slot];
}


ItemStack *InventoryPlayer::getStackInSlotOnClosing(int_t i)
{
    if (i < 0 || i >= getSizeInventory())
        return nullptr;
    ItemStack **inventory = mainInventory;
    if (i >= 36)
    {
        i -= 36;
        inventory = armorInventory;
    }
    if (inventory[i] == nullptr)
        return nullptr;
    ItemStack *stack = inventory[i];
    inventory[i] = nullptr;
    return stack;
}

ItemStack* InventoryPlayer::decrStackSize(int_t slot, int_t amount) {
    if (slot < 0 || slot >= getSizeInventory() || amount <= 0) {
        return nullptr;
    }
    ItemStack** inventory = mainInventory;
    if (slot >= 36) {
        inventory = armorInventory;
        slot -= 36;
    }
    if (inventory[slot] != nullptr) {
        if (inventory[slot]->stackSize <= amount) {
            ItemStack* stack = inventory[slot];
            inventory[slot] = nullptr;
            return stack;
        }
        ItemStack* split = inventory[slot]->splitStack(amount);
        if (inventory[slot]->stackSize == 0) {
            delete inventory[slot];
            inventory[slot] = nullptr;
        }
        return split;
    }
    return nullptr;
}

void InventoryPlayer::setInventorySlotContents(int_t slot, ItemStack* stack) {
    if (slot < 0 || slot >= getSizeInventory()) {
        delete stack;
        return;
    }
    ItemStack** inventory = mainInventory;
    if (slot >= 36) {
        slot -= 36;
        inventory = armorInventory;
    }
    if (inventory[slot] != stack)
        delete inventory[slot];
    inventory[slot] = stack;
}

std::string InventoryPlayer::getInvName() {
    return "container.inventory";
}

int_t InventoryPlayer::getInventoryStackLimit() {
    return 64;
}

void InventoryPlayer::onInventoryChanged() {
    inventoryChanged = true;
}

bool InventoryPlayer::canInteractWith(EntityPlayer* entityplayer) {
    if (player == nullptr || player->isDead) {
        return false;
    }
    return entityplayer != nullptr && entityplayer->getDistanceSqToEntity(player) <= 64.0;
}

float InventoryPlayer::getStrVsBlock(Block* block) {
    float strength = 1.0f;
    ItemStack* stack = getCurrentItem();
    if (stack != nullptr) {
        strength *= stack->getStrVsBlock(block);
    }
    return strength;
}

NBTTagList* InventoryPlayer::writeToNBT(NBTTagList* tagList) {
    for (int i = 0; i < 36; i++) {
        if (mainInventory[i] != nullptr && mainInventory[i]->isValid()) {
            NBTTagCompound* tag = new NBTTagCompound();
            tag->setByte("Slot", (char)i);
            mainInventory[i]->writeToNBT(tag);
            tagList->setTag(tag);
        }
    }
    for (int i = 0; i < 4; i++) {
        if (armorInventory[i] != nullptr && armorInventory[i]->isValid()) {
            NBTTagCompound* tag = new NBTTagCompound();
            tag->setByte("Slot", (char)(i + 100));
            armorInventory[i]->writeToNBT(tag);
            tagList->setTag(tag);
        }
    }
    return tagList;
}

void InventoryPlayer::readFromNBT(NBTTagList* tagList) {
    for (int i = 0; i < 36; i++) {
        delete mainInventory[i];
        mainInventory[i] = nullptr;
    }
    for (int i = 0; i < 4; i++) {
        delete armorInventory[i];
        armorInventory[i] = nullptr;
    }
    for (int i = 0; i < tagList->tagCount(); i++) {
        NBTTagCompound* tag = dynamic_cast<NBTTagCompound*>(tagList->tagAt(i));
        if (tag == nullptr) {
            continue;
        }
        int slot = tag->getByte("Slot") & 0xff;
        ItemStack* stack = ItemStack::loadItemStackFromNBT(tag);
        if (stack == nullptr) {
            continue;
        }
        if (slot >= 0 && slot < 36) {
            delete mainInventory[slot];
            mainInventory[slot] = stack;
        } else if (slot >= 100 && slot < 104) {
            delete armorInventory[slot - 100];
            armorInventory[slot - 100] = stack;
        } else {
            delete stack;
        }
    }
}

int InventoryPlayer::getDamageVsEntity(Entity* entity) {
    ItemStack* stack = getCurrentItem();
    if (stack != nullptr) {
        return stack->getDamageVsEntity(entity);
    }
    return 1;
}

bool InventoryPlayer::canHarvestBlock(Block* block) {
    if (block->blockMaterial->getIsHarvestable()) {
        return true;
    }
    ItemStack* stack = getCurrentItem();
    if (stack != nullptr) {
        return stack->canHarvestBlock(block);
    }
    return false;
}

ItemStack* InventoryPlayer::armorItemInSlot(int slot) {
    return slot >= 0 && slot < 4 ? armorInventory[slot] : nullptr;
}

int InventoryPlayer::getTotalArmorValue() {
    int total = 0;
    for (int i = 0; i < 4; i++) {
        if (armorInventory[i] != nullptr && dynamic_cast<ItemArmor*>(armorInventory[i]->getItem()) != nullptr) {
            total += static_cast<ItemArmor*>(armorInventory[i]->getItem())->damageReduceAmount;
        }
    }
    return total;
}

void InventoryPlayer::damageArmor(int amount) {
    amount /= 4;
    if (amount < 1) {
        amount = 1;
    }
    for (int j = 0; j < 4; j++) {
        if (armorInventory[j] == nullptr || dynamic_cast<ItemArmor*>(armorInventory[j]->getItem()) == nullptr) {
            continue;
        }
        armorInventory[j]->damageItem(amount, player);
        if (armorInventory[j]->stackSize == 0) {
            armorInventory[j]->onItemDestroyedByUse(player);
            delete armorInventory[j];
            armorInventory[j] = nullptr;
        }
    }
}

void InventoryPlayer::dropAllItems() {
    for (int i = 0; i < 36; i++) {
        if (mainInventory[i] != nullptr) {
            player->dropPlayerItemWithRandomChoice(mainInventory[i], true);
            mainInventory[i] = nullptr;
        }
    }
    for (int j = 0; j < 4; j++) {
        if (armorInventory[j] != nullptr) {
            player->dropPlayerItemWithRandomChoice(armorInventory[j], true);
            armorInventory[j] = nullptr;
        }
    }
}

void InventoryPlayer::setItemStack(ItemStack* stack) {
    itemStack = stack;
    if (player != nullptr) {
        player->onItemStackChanged(stack);
    }
}

ItemStack* InventoryPlayer::getItemStack() {
    return itemStack;
}

bool InventoryPlayer::isUseableByPlayer(EntityPlayer* entityplayer) const {
    return player != nullptr && entityplayer != nullptr && !player->isDead &&
           entityplayer->getDistanceSqToEntity(player) <= 64.0;
}

bool InventoryPlayer::hasItemStack(ItemStack* stack) const {
    if (stack == nullptr) {
        return false;
    }
    for (int i = 0; i < 4; ++i) {
        if (armorInventory[i] != nullptr && armorInventory[i]->isStackEqual(stack)) {
            return true;
        }
    }
    for (int i = 0; i < 36; ++i) {
        if (mainInventory[i] != nullptr && mainInventory[i]->isStackEqual(stack)) {
            return true;
        }
    }
    return false;
}

void InventoryPlayer::openChest() {
}

void InventoryPlayer::closeChest() {
}

bool InventoryPlayer::doesXZShowFog(ItemStack* stack) {
    return hasItemStack(stack);
}

void InventoryPlayer::copyInventory(const InventoryPlayer* other)
{
    if (other == nullptr)
        return;

    for (int i = 0; i < 36; ++i)
    {
        delete mainInventory[i];
        mainInventory[i] = ItemStack::copyItemStack(other->mainInventory[i]);
    }
    for (int i = 0; i < 4; ++i)
    {
        delete armorInventory[i];
        armorInventory[i] = ItemStack::copyItemStack(other->armorInventory[i]);
    }
    inventoryChanged = true;
}
