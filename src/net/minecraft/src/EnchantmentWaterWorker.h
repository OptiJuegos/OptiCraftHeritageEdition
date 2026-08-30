#pragma once
#include "Enchantment.h"
class EnchantmentWaterWorker : public Enchantment
{
public:
    EnchantmentWaterWorker(int id, int weight);
    int getMinEnchantability(int level) const override;
    int getMaxEnchantability(int level) const override;
    int getMaxLevel() const override;
};
