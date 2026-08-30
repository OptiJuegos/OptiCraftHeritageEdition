#include "ItemBow.h"

#include "Enchantment.h"
#include "EnchantmentHelper.h"
#include "EntityArrow.h"
#include "EntityPlayer.h"
#include "InventoryPlayer.h"
#include "ItemStack.h"
#include "World.h"

ItemBow::ItemBow(int id) : Item(id)
{
    maxStackSize = 1;
    setMaxDamage(384);
}

ItemStack* ItemBow::onFoodEaten(ItemStack* itemstack, World*, EntityPlayer*)
{
    return itemstack;
}

void ItemBow::onPlayerStoppedUsing(ItemStack* itemstack, World* world, EntityPlayer* entityplayer, int timeLeft)
{
    const bool infinite = entityplayer->capabilities.isCreativeMode ||
        (Enchantment::infinity != nullptr && EnchantmentHelper::getEnchantmentLevel(Enchantment::infinity->effectId, itemstack) > 0);
    if (!infinite && entityplayer->inventory->getInventorySlotContainItem(Item::arrow->shiftedIndex) < 0)
        return;

    const int charge = getMaxItemUseDuration(itemstack) - timeLeft;
    float strength = static_cast<float>(charge) / 20.0f;
    strength = (strength * strength + strength * 2.0f) / 3.0f;
    if (strength < 0.1f)
        return;
    if (strength > 1.0f)
        strength = 1.0f;

    EntityArrow* arrow = new EntityArrow(world, entityplayer, strength * 2.0f);
    if (strength == 1.0f)
        arrow->arrowCritical = true;

    if (Enchantment::power != nullptr)
    {
        const int power = EnchantmentHelper::getEnchantmentLevel(Enchantment::power->effectId, itemstack);
        if (power > 0)
            arrow->setDamage(arrow->getDamage() + static_cast<double>(power) * 0.5 + 0.5);
    }
    if (Enchantment::punch != nullptr)
    {
        const int punch = EnchantmentHelper::getEnchantmentLevel(Enchantment::punch->effectId, itemstack);
        if (punch > 0)
            arrow->setKnockbackStrength(punch);
    }
    if (Enchantment::flame != nullptr && EnchantmentHelper::getEnchantmentLevel(Enchantment::flame->effectId, itemstack) > 0)
        arrow->setFire(100);

    itemstack->damageItem(1, entityplayer);
    world->playSoundAtEntity(entityplayer, "random.bow", 1.0f,
        1.0f / (itemRand.nextFloat() * 0.4f + 1.2f) + strength * 0.5f);

    if (!infinite)
        entityplayer->inventory->consumeInventoryItem(Item::arrow->shiftedIndex);
    else
        arrow->doesArrowBelongToPlayer = false;

    if (!world->multiplayerWorld)
    {
        if (!world->entityJoinedWorld(arrow))
            delete arrow;
    }
    else
        delete arrow;
}

int ItemBow::getMaxItemUseDuration(ItemStack*) { return 72000; }
EnumAction ItemBow::getItemUseAction(ItemStack*) { return EnumAction::bow; }

ItemStack* ItemBow::onItemRightClick(ItemStack* itemstack, World*, EntityPlayer* entityplayer)
{
    if (entityplayer->capabilities.isCreativeMode || entityplayer->inventory->getInventorySlotContainItem(Item::arrow->shiftedIndex) >= 0)
        entityplayer->setItemInUse(itemstack, getMaxItemUseDuration(itemstack));
    return itemstack;
}

int ItemBow::getItemEnchantability() { return 1; }
