#pragma once

#include <vector>

#include "EnchantmentData.h"

class DamageSource;
class EntityLiving;
class InventoryPlayer;
class ItemStack;
class Random;

class EnchantmentHelper
{
public:
    static int getEnchantmentLevel(int effectId, ItemStack *stack);
    static int getEnchantmentModifierDamage(InventoryPlayer *inventory, const DamageSource &source);
    static int getEnchantmentModifierLiving(InventoryPlayer *inventory, EntityLiving *living);
    static int getKnockbackModifier(InventoryPlayer *inventory, EntityLiving *living);
    static int getFireAspectModifier(InventoryPlayer *inventory, EntityLiving *living);
    static int getRespiration(InventoryPlayer *inventory);
    static int getEfficiencyModifier(InventoryPlayer *inventory);
    static int getUnbreakingModifier(InventoryPlayer *inventory);
    static bool getSilkTouchModifier(InventoryPlayer *inventory);
    static int getFortuneModifier(InventoryPlayer *inventory);
    static int getLootingModifier(InventoryPlayer *inventory);
    static bool getAquaAffinityModifier(InventoryPlayer *inventory);

    static int calcItemStackEnchantability(Random &random, int slot, int bookshelves, ItemStack *stack);
    static void addRandomEnchantment(Random &random, ItemStack *stack, int level);
    static std::vector<EnchantmentData> buildEnchantmentList(Random &random, ItemStack *stack, int level);
    static std::vector<EnchantmentData> mapEnchantmentData(int enchantability, ItemStack *stack);

private:
    static int getMaxEnchantmentLevel(int effectId, ItemStack **stacks, int count);
};
