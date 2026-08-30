#include "ItemPotion.h"

#include <memory>

#include "EntityPlayer.h"
#include "EntityPotion.h"
#include "InventoryPlayer.h"
#include "ItemStack.h"
#include "Potion.h"
#include "PotionHelper.h"
#include "StatCollector.h"
#include "World.h"

ItemPotion::ItemPotion(int id) : Item(id)
{
    setMaxStackSize(1);
    setHasSubtypes(true);
    setMaxDamage(0);
}

const std::vector<PotionEffect> &ItemPotion::getEffects(ItemStack *itemstack)
{
    return getEffects(itemstack != nullptr ? itemstack->getItemDamage() : 0);
}

const std::vector<PotionEffect> &ItemPotion::getEffects(int damage)
{
    auto it = effectCache.find(damage);
    if (it == effectCache.end())
        it = effectCache.emplace(damage, PotionHelper::getPotionEffects(damage, false)).first;
    return it->second;
}

ItemStack *ItemPotion::onFoodEaten(ItemStack *itemstack, World *world, EntityPlayer *player)
{
    --itemstack->stackSize;
    if (!world->multiplayerWorld)
    {
        const auto &effects = getEffects(itemstack);
        for (const PotionEffect &effect : effects)
            player->addPotionEffect(new PotionEffect(effect));
    }

    if (itemstack->stackSize <= 0)
        return new ItemStack(Item::glassBottle);

    std::unique_ptr<ItemStack> bottle(new ItemStack(Item::glassBottle));
    if (!player->inventory->addItemStackToInventory(bottle.get()))
        player->dropPlayerItem(bottle.release());
    return itemstack;
}

int ItemPotion::getMaxItemUseDuration(ItemStack *)
{
    return 32;
}

EnumAction ItemPotion::getItemUseAction(ItemStack *)
{
    return EnumAction::drink;
}

ItemStack *ItemPotion::onItemRightClick(ItemStack *itemstack, World *world, EntityPlayer *player)
{
    if (isSplash(itemstack->getItemDamage()))
    {
        --itemstack->stackSize;
        world->playSoundAtEntity(player, "random.bow", 0.5f, 0.4f / (itemRand.nextFloat() * 0.4f + 0.8f));
        if (!world->multiplayerWorld)
        {
            EntityPotion *potion = new EntityPotion(world, player, itemstack->getItemDamage());
            if (!world->spawnEntityInWorld(potion))
                delete potion;
        }
        return itemstack;
    }

    player->setItemInUse(itemstack, getMaxItemUseDuration(itemstack));
    return itemstack;
}

bool ItemPotion::onItemUse(ItemStack *, EntityPlayer *, World *, int, int, int, int)
{
    return false;
}

int ItemPotion::getIconFromDamage(int damage)
{
    return isSplash(damage) ? 154 : 140;
}

int ItemPotion::func_46057_a(int damage, int renderPass)
{
    return renderPass == 0 ? 141 : Item::func_46057_a(damage, renderPass);
}

bool ItemPotion::isSplash(int damage)
{
    return (damage & 16384) != 0;
}

int ItemPotion::getColorFromDamage(int damage)
{
    return PotionHelper::func_40358_a(damage, false);
}

int ItemPotion::getColorFromDamage(int damage, int renderPass)
{
    return renderPass > 0 ? 0xffffff : PotionHelper::func_40358_a(damage, false);
}

bool ItemPotion::func_46058_c()
{
    return true;
}

bool ItemPotion::isEffectInstant(int damage)
{
    const auto &effects = getEffects(damage);
    for (const PotionEffect &effect : effects)
    {
        Potion *potion = Potion::getPotion(effect.getPotionID());
        if (potion != nullptr && potion->isInstant())
            return true;
    }
    return false;
}

bool ItemPotion::hasEffect(ItemStack *itemstack)
{
    return itemstack != nullptr && !getEffects(itemstack).empty();
}

std::string ItemPotion::getItemDisplayName(ItemStack *itemstack)
{
    if (itemstack == nullptr || itemstack->getItemDamage() == 0)
        return StatCollector::translateToLocal("item.emptyPotion.name");

    std::string prefix;
    if (isSplash(itemstack->getItemDamage()))
        prefix = StatCollector::translateToLocal("potion.prefix.grenade") + " ";

    const std::vector<PotionEffect>& effects = getEffects(itemstack);
    if (!effects.empty())
        return prefix + StatCollector::translateToLocal(effects.front().getEffectName() + ".postfix");

    return StatCollector::translateToLocal(PotionHelper::func_40359_b(itemstack->getItemDamage())) +
           " " + Item::getItemDisplayName(itemstack);
}

void ItemPotion::addInformation(ItemStack *itemstack, std::vector<std::string>& information)
{
    if (itemstack == nullptr || itemstack->getItemDamage() == 0)
        return;

    const std::vector<PotionEffect>& effects = getEffects(itemstack);
    if (effects.empty())
    {
        information.push_back(u8"§7" + StatCollector::translateToLocal("potion.empty"));
        return;
    }

    Potion::initPotions();
    for (const PotionEffect& effect : effects)
    {
        std::string line = StatCollector::translateToLocal(effect.getEffectName());
        if (effect.getAmplifier() > 0)
            line += " " + StatCollector::translateToLocal("potion.potency." + std::to_string(effect.getAmplifier()));
        if (effect.getDuration() > 20)
            line += " (" + Potion::getDurationString(effect) + ")";

        Potion *potion = Potion::getPotion(effect.getPotionID());
        information.push_back(std::string(potion != nullptr && potion->getIsBadEffect() ? u8"§c" : u8"§7") + line);
    }
}
