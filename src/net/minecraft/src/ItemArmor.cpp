#include "ItemArmor.h"

const int ItemArmor::maxDamageArray[4] = { 11, 16, 15, 13 };

ItemArmor::ItemArmor(int id, EnumArmorMaterial armorMaterial, int render, int type)
    : Item(id), material(armorMaterial), armorType(type),
      damageReduceAmount(EnumArmorMaterialHelper::getDamageReductionAmount(armorMaterial, type)),
      renderIndex(render)
{
    setMaxDamage(EnumArmorMaterialHelper::getDurability(armorMaterial, type));
    maxStackSize = 1;
}

int ItemArmor::getItemEnchantability()
{
    return EnumArmorMaterialHelper::getEnchantability(material);
}
