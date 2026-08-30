#pragma once

enum class EnumToolMaterial {
    WOOD,
    STONE,
    IRON,
    EMERALD,
    GOLD
};

namespace EnumToolMaterialHelper {
    int getMaxUses(EnumToolMaterial material);
    float getEfficiencyOnProperMaterial(EnumToolMaterial material);
    int getDamageVsEntity(EnumToolMaterial material);
    int getHarvestLevel(EnumToolMaterial material);
    int getEnchantability(EnumToolMaterial material);
}
