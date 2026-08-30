#pragma once

#include <string>
#include <vector>

#include "PotionEffect.h"

// net.minecraft.src.PotionHelper
class PotionHelper
{
public:
    static bool checkFlag(int value, int bit);
    static int func_40352_a(int value);
    static int func_40354_a(const std::vector<PotionEffect> &effects);
    static int func_40358_a(int value, bool includeUnusable);
    static std::string func_40359_b(int value);
    static std::vector<PotionEffect> getPotionEffects(int value, bool includeUnusable);
    static int applyIngredient(int value, const std::string &ingredient);
    static int func_40351_a(int value, int bit1, int bit2, int bit3, int bit4, int bit5);

    static const std::string sugarEffect;
    static const std::string ghastTearEffect;
    static const std::string spiderEyeEffect;
    static const std::string fermentedSpiderEyeEffect;
    static const std::string speckledMelonEffect;
    static const std::string blazePowderEffect;
    static const std::string magmaCreamEffect;
    static const std::string redstoneEffect;
    static const std::string glowstoneEffect;
    static const std::string gunpowderEffect;

private:
    static void ensureInitialized();
    static int isFlagSet(int value, int bit);
    static int isFlagUnset(int value, int bit);
    static int func_40347_a(bool invert, bool multiply, bool negative, int comparison, int bit, int multiplier, int value);
    static int countSetFlags(int value);
    static int func_40355_a(const std::string &expression, int start, int end, int value);
    static int brewBitOperations(int value, int bit, bool clear, bool toggle, bool requireSet);
};
