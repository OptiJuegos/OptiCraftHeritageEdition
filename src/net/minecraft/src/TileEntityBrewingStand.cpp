#include "TileEntityBrewingStand.h"

#include <vector>

#include "EntityPlayer.h"
#include "Item.h"
#include "ItemPotion.h"
#include "ItemStack.h"
#include "NBTTagCompound.h"
#include "NBTTagList.h"
#include "PotionEffect.h"
#include "PotionHelper.h"
#include "World.h"

namespace
{
bool potionEffectsEqual(const std::vector<PotionEffect> &a, const std::vector<PotionEffect> &b)
{
    if (a.size() != b.size())
        return false;
    for (std::size_t i = 0; i < a.size(); ++i)
    {
        if (!a[i].equals(b[i]))
            return false;
    }
    return true;
}
}

TileEntityBrewingStand::TileEntityBrewingStand()
{
    for (ItemStack *&stack : brewingItemStacks)
        stack = nullptr;
}

TileEntityBrewingStand::~TileEntityBrewingStand()
{
    for (ItemStack *&stack : brewingItemStacks)
    {
        delete stack;
        stack = nullptr;
    }
}

std::string TileEntityBrewingStand::getInvName()
{
    return "container.brewing";
}

int_t TileEntityBrewingStand::getSizeInventory()
{
    return 4;
}

void TileEntityBrewingStand::updateEntity()
{
    if (brewTime > 0)
    {
        --brewTime;
        if (brewTime == 0)
        {
            brewPotions();
            onInventoryChanged();
        }
        else if (!canBrew())
        {
            brewTime = 0;
            onInventoryChanged();
        }
        else if (brewingItemStacks[3] == nullptr || ingredientID != brewingItemStacks[3]->itemID)
        {
            brewTime = 0;
            onInventoryChanged();
        }
    }
    else if (canBrew())
    {
        brewTime = 400;
        ingredientID = brewingItemStacks[3]->itemID;
    }

    const int_t newFilledSlots = getFilledSlots();
    if (newFilledSlots != filledSlots)
    {
        filledSlots = newFilledSlots;
        if (worldObj != nullptr)
            worldObj->setBlockMetadataWithNotify(xCoord, yCoord, zCoord, filledSlots);
    }

    TileEntity::updateEntity();
}

int_t TileEntityBrewingStand::getBrewTime() const
{
    return brewTime;
}

bool TileEntityBrewingStand::canBrew() const
{
    ItemStack *ingredient = brewingItemStacks[3];
    if (ingredient == nullptr || ingredient->stackSize <= 0 || !ItemStack::isValidItemID(ingredient->itemID))
        return false;

    Item *ingredientItem = Item::itemsList[ingredient->itemID];
    if (ingredientItem == nullptr || !ingredientItem->isPotionIngredient())
        return false;

    ItemPotion *potionItem = dynamic_cast<ItemPotion *>(Item::potion);
    if (potionItem == nullptr)
        return false;

    for (int i = 0; i < 3; ++i)
    {
        ItemStack *stack = brewingItemStacks[i];
        if (stack == nullptr || stack->itemID != Item::potion->shiftedIndex)
            continue;

        const int oldDamage = stack->getItemDamage();
        const int newDamage = getPotionResult(oldDamage, ingredient);
        if (!ItemPotion::isSplash(oldDamage) && ItemPotion::isSplash(newDamage))
            return true;

        const auto &oldEffects = potionItem->getEffects(oldDamage);
        const auto &newEffects = potionItem->getEffects(newDamage);
        if (oldDamage != newDamage &&
            (oldEffects.empty() || (!potionEffectsEqual(oldEffects, newEffects) && !newEffects.empty())))
            return true;
    }
    return false;
}

void TileEntityBrewingStand::brewPotions()
{
    if (!canBrew())
        return;

    ItemStack *ingredient = brewingItemStacks[3];
    ItemPotion *potionItem = dynamic_cast<ItemPotion *>(Item::potion);
    if (ingredient == nullptr || potionItem == nullptr)
        return;

    for (int i = 0; i < 3; ++i)
    {
        ItemStack *stack = brewingItemStacks[i];
        if (stack == nullptr || stack->itemID != Item::potion->shiftedIndex)
            continue;

        const int oldDamage = stack->getItemDamage();
        const int newDamage = getPotionResult(oldDamage, ingredient);
        const auto &oldEffects = potionItem->getEffects(oldDamage);
        const auto &newEffects = potionItem->getEffects(newDamage);
        const bool sameCachedPotion = oldDamage > 0 && oldDamage == newDamage;
        const bool preservesOrRemovesEffects = !oldEffects.empty() &&
                                               (potionEffectsEqual(oldEffects, newEffects) || newEffects.empty());
        if (sameCachedPotion || preservesOrRemovesEffects)
        {
            if (!ItemPotion::isSplash(oldDamage) && ItemPotion::isSplash(newDamage))
                stack->setItemDamage(newDamage);
        }
        else if (oldDamage != newDamage)
        {
            stack->setItemDamage(newDamage);
        }
    }

    Item *ingredientItem = ItemStack::isValidItemID(ingredient->itemID) ? Item::itemsList[ingredient->itemID] : nullptr;
    if (ingredientItem != nullptr && ingredientItem->hasContainerItem())
    {
        Item *containerItem = ingredientItem->getContainerItem();
        delete brewingItemStacks[3];
        brewingItemStacks[3] = new ItemStack(containerItem);
    }
    else
    {
        --ingredient->stackSize;
        if (ingredient->stackSize <= 0)
        {
            delete brewingItemStacks[3];
            brewingItemStacks[3] = nullptr;
        }
    }
}

int_t TileEntityBrewingStand::getPotionResult(int_t damage, ItemStack *ingredient) const
{
    if (ingredient == nullptr || !ItemStack::isValidItemID(ingredient->itemID))
        return damage;
    Item *item = Item::itemsList[ingredient->itemID];
    if (item == nullptr || !item->isPotionIngredient())
        return damage;
    return PotionHelper::applyIngredient(damage, item->getPotionEffect());
}

void TileEntityBrewingStand::readFromNBT(NBTTagCompound *compound)
{
    TileEntity::readFromNBT(compound);
    for (ItemStack *&stack : brewingItemStacks)
    {
        delete stack;
        stack = nullptr;
    }

    NBTTagList *items = compound->getTagList("Items");
    if (items != nullptr)
    {
        for (int i = 0; i < items->tagCount(); ++i)
        {
            NBTTagCompound *itemTag = dynamic_cast<NBTTagCompound *>(items->tagAt(i));
            if (itemTag == nullptr)
                continue;
            const int slot = itemTag->getByte("Slot") & 255;
            if (slot >= 0 && slot < 4)
            {
                ItemStack *loaded = new ItemStack(itemTag);
                delete brewingItemStacks[slot];
                brewingItemStacks[slot] = loaded;
            }
        }
    }
    brewTime = compound->getShort("BrewTime");
}

void TileEntityBrewingStand::writeToNBT(NBTTagCompound *compound)
{
    TileEntity::writeToNBT(compound);
    compound->setShort("BrewTime", static_cast<short_t>(brewTime));
    NBTTagList *items = new NBTTagList();
    for (int i = 0; i < 4; ++i)
    {
        if (brewingItemStacks[i] == nullptr)
            continue;
        NBTTagCompound *itemTag = new NBTTagCompound();
        itemTag->setByte("Slot", static_cast<byte_t>(i));
        brewingItemStacks[i]->writeToNBT(itemTag);
        items->appendTag(itemTag);
    }
    compound->setTag("Items", items);
}

ItemStack *TileEntityBrewingStand::getStackInSlot(int_t slot)
{
    return slot >= 0 && slot < 4 ? brewingItemStacks[slot] : nullptr;
}

ItemStack *TileEntityBrewingStand::decrStackSize(int_t slot, int_t)
{
    if (slot < 0 || slot >= 4)
        return nullptr;
    ItemStack *stack = brewingItemStacks[slot];
    brewingItemStacks[slot] = nullptr;
    return stack;
}

ItemStack *TileEntityBrewingStand::getStackInSlotOnClosing(int_t slot)
{
    if (slot < 0 || slot >= 4)
        return nullptr;
    ItemStack *stack = brewingItemStacks[slot];
    brewingItemStacks[slot] = nullptr;
    return stack;
}

void TileEntityBrewingStand::setInventorySlotContents(int_t slot, ItemStack *itemstack)
{
    if (slot < 0 || slot >= 4)
    {
        delete itemstack;
        return;
    }
    if (brewingItemStacks[slot] != itemstack)
        delete brewingItemStacks[slot];
    brewingItemStacks[slot] = itemstack;
}

int_t TileEntityBrewingStand::getInventoryStackLimit()
{
    return 1;
}

bool TileEntityBrewingStand::canInteractWith(EntityPlayer *player)
{
    if (worldObj == nullptr || worldObj->getBlockTileEntity(xCoord, yCoord, zCoord) != this)
        return false;
    return player->getDistanceSq(static_cast<double>(xCoord) + 0.5,
                                 static_cast<double>(yCoord) + 0.5,
                                 static_cast<double>(zCoord) + 0.5) <= 64.0;
}

void TileEntityBrewingStand::setBrewTime(int_t time)
{
    brewTime = time;
}

int_t TileEntityBrewingStand::getFilledSlots() const
{
    int_t result = 0;
    for (int i = 0; i < 3; ++i)
    {
        if (brewingItemStacks[i] != nullptr)
            result |= 1 << i;
    }
    return result;
}
