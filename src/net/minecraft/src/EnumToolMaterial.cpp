#include "EnumToolMaterial.h"

int EnumToolMaterialHelper::getMaxUses(EnumToolMaterial material) {
    switch (material) {
        case EnumToolMaterial::WOOD: return 59;
        case EnumToolMaterial::STONE: return 131;
        case EnumToolMaterial::IRON: return 250;
        case EnumToolMaterial::EMERALD: return 1561;
        case EnumToolMaterial::GOLD: return 32;
        default: return 0;
    }
}

float EnumToolMaterialHelper::getEfficiencyOnProperMaterial(EnumToolMaterial material) {
    switch (material) {
        case EnumToolMaterial::WOOD: return 2.0f;
        case EnumToolMaterial::STONE: return 4.0f;
        case EnumToolMaterial::IRON: return 6.0f;
        case EnumToolMaterial::EMERALD: return 8.0f;
        case EnumToolMaterial::GOLD: return 12.0f;
        default: return 0.0f;
    }
}

int EnumToolMaterialHelper::getDamageVsEntity(EnumToolMaterial material) {
    switch (material) {
        case EnumToolMaterial::WOOD: return 0;
        case EnumToolMaterial::STONE: return 1;
        case EnumToolMaterial::IRON: return 2;
        case EnumToolMaterial::EMERALD: return 3;
        case EnumToolMaterial::GOLD: return 0;
        default: return 0;
    }
}

int EnumToolMaterialHelper::getHarvestLevel(EnumToolMaterial material) {
    switch (material) {
        case EnumToolMaterial::WOOD: return 0;
        case EnumToolMaterial::STONE: return 1;
        case EnumToolMaterial::IRON: return 2;
        case EnumToolMaterial::EMERALD: return 3;
        case EnumToolMaterial::GOLD: return 0;
        default: return 0;
    }
}

int EnumToolMaterialHelper::getEnchantability(EnumToolMaterial material) {
    switch (material) {
        case EnumToolMaterial::WOOD: return 15;
        case EnumToolMaterial::STONE: return 5;
        case EnumToolMaterial::IRON: return 14;
        case EnumToolMaterial::EMERALD: return 10;
        case EnumToolMaterial::GOLD: return 22;
        default: return 0;
    }
}
