#include "EnumArmorMaterial.h"

namespace
{
const int kBaseDurability[4] = { 11, 16, 15, 13 };
}

int EnumArmorMaterialHelper::getDurability(EnumArmorMaterial material, int armorType)
{
    int factor = 0;
    switch (material)
    {
    case EnumArmorMaterial::CLOTH: factor = 5; break;
    case EnumArmorMaterial::CHAIN: factor = 15; break;
    case EnumArmorMaterial::IRON: factor = 15; break;
    case EnumArmorMaterial::GOLD: factor = 7; break;
    case EnumArmorMaterial::DIAMOND: factor = 33; break;
    }
    return kBaseDurability[armorType] * factor;
}

int EnumArmorMaterialHelper::getDamageReductionAmount(EnumArmorMaterial material, int armorType)
{
    static const int cloth[4] = { 1, 3, 2, 1 };
    static const int chain[4] = { 2, 5, 4, 1 };
    static const int iron[4] = { 2, 6, 5, 2 };
    static const int gold[4] = { 2, 5, 3, 1 };
    static const int diamond[4] = { 3, 8, 6, 3 };
    const int *values = cloth;
    switch (material)
    {
    case EnumArmorMaterial::CLOTH: values = cloth; break;
    case EnumArmorMaterial::CHAIN: values = chain; break;
    case EnumArmorMaterial::IRON: values = iron; break;
    case EnumArmorMaterial::GOLD: values = gold; break;
    case EnumArmorMaterial::DIAMOND: values = diamond; break;
    }
    return values[armorType];
}

int EnumArmorMaterialHelper::getEnchantability(EnumArmorMaterial material)
{
    switch (material)
    {
    case EnumArmorMaterial::CLOTH: return 15;
    case EnumArmorMaterial::CHAIN: return 12;
    case EnumArmorMaterial::IRON: return 9;
    case EnumArmorMaterial::GOLD: return 25;
    case EnumArmorMaterial::DIAMOND: return 10;
    }
    return 0;
}
