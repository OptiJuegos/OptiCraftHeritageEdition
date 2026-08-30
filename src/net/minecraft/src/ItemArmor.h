#pragma once

#include "Item.h"
#include "EnumArmorMaterial.h"

class ItemArmor : public Item {
public:
    ItemArmor(int id, EnumArmorMaterial material, int renderIndex, int armorType);

    static const int maxDamageArray[4];

    int getItemEnchantability() override;

    EnumArmorMaterial material;
    int armorType;
    int damageReduceAmount;
    int renderIndex;
};
