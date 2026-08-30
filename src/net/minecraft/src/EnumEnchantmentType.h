#pragma once

class Item;

enum class EnumEnchantmentType
{
    all,
    armor,
    armor_feet,
    armor_legs,
    armor_torso,
    armor_head,
    weapon,
    digger,
    bow
};

namespace EnumEnchantmentTypeHelper
{
    bool canEnchantItem(EnumEnchantmentType type, Item *item);
}
