#include "EnumEnchantmentType.h"

#include "Item.h"
#include "ItemArmor.h"
#include "ItemBow.h"
#include "ItemSword.h"
#include "ItemTool.h"

bool EnumEnchantmentTypeHelper::canEnchantItem(EnumEnchantmentType type, Item *item)
{
    if (item == nullptr)
        return false;
    if (type == EnumEnchantmentType::all)
        return true;

    if (ItemArmor *armor = dynamic_cast<ItemArmor *>(item))
    {
        if (type == EnumEnchantmentType::armor)
            return true;
        switch (armor->armorType)
        {
        case 0: return type == EnumEnchantmentType::armor_head;
        case 1: return type == EnumEnchantmentType::armor_torso;
        case 2: return type == EnumEnchantmentType::armor_legs;
        case 3: return type == EnumEnchantmentType::armor_feet;
        default: return false;
        }
    }

    if (dynamic_cast<ItemSword *>(item) != nullptr)
        return type == EnumEnchantmentType::weapon;
    if (dynamic_cast<ItemTool *>(item) != nullptr)
        return type == EnumEnchantmentType::digger;
    if (dynamic_cast<ItemBow *>(item) != nullptr)
        return type == EnumEnchantmentType::bow;
    return false;
}
