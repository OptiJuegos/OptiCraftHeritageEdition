#pragma once

enum class EnumArmorMaterial
{
    CLOTH,
    CHAIN,
    IRON,
    GOLD,
    DIAMOND
};

namespace EnumArmorMaterialHelper
{
    int getDurability(EnumArmorMaterial material, int armorType);
    int getDamageReductionAmount(EnumArmorMaterial material, int armorType);
    int getEnchantability(EnumArmorMaterial material);
}
