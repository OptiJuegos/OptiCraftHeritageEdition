#include "PotionHelper.h"
#include "java/Arithmetic.h"

#include <cmath>
#include <unordered_map>

#include "Potion.h"

const std::string PotionHelper::sugarEffect = "-0+1-2-3&4-4+13";
const std::string PotionHelper::ghastTearEffect = "+0-1-2-3&4-4+13";
const std::string PotionHelper::spiderEyeEffect = "-0-1+2-3&4-4+13";
const std::string PotionHelper::fermentedSpiderEyeEffect = "-0+3-4+13";
const std::string PotionHelper::speckledMelonEffect = "+0-1+2-3&4-4+13";
const std::string PotionHelper::blazePowderEffect = "+0-1-2+3&4-4+13";
const std::string PotionHelper::magmaCreamEffect = "+0+1-2-3&4-4+13";
const std::string PotionHelper::redstoneEffect = "-5+6-7";
const std::string PotionHelper::glowstoneEffect = "+5-6-7";
const std::string PotionHelper::gunpowderEffect = "+14&13-13";

namespace
{
std::unordered_map<int, std::string> potionRequirements;
std::unordered_map<int, std::string> potionAmplifiers;
std::unordered_map<int, int> potionColorCache;
bool potionHelperInitialized = false;

const char *potionPrefixes[] = {
    "potion.prefix.mundane", "potion.prefix.uninteresting", "potion.prefix.bland", "potion.prefix.clear",
    "potion.prefix.milky", "potion.prefix.diffuse", "potion.prefix.artless", "potion.prefix.thin",
    "potion.prefix.awkward", "potion.prefix.flat", "potion.prefix.bulky", "potion.prefix.bungling",
    "potion.prefix.buttered", "potion.prefix.smooth", "potion.prefix.suave", "potion.prefix.debonair",
    "potion.prefix.thick", "potion.prefix.elegant", "potion.prefix.fancy", "potion.prefix.charming",
    "potion.prefix.dashing", "potion.prefix.refined", "potion.prefix.cordial", "potion.prefix.sparkling",
    "potion.prefix.potent", "potion.prefix.foul", "potion.prefix.odorless", "potion.prefix.rank",
    "potion.prefix.harsh", "potion.prefix.acrid", "potion.prefix.gross", "potion.prefix.stinky"
};
}

void PotionHelper::ensureInitialized()
{
    if (potionHelperInitialized)
        return;

    Potion::initPotions();
    potionRequirements[Potion::regeneration->getId()] = "0 & !1 & !2 & !3 & 0+6";
    potionRequirements[Potion::moveSpeed->getId()] = "!0 & 1 & !2 & !3 & 1+6";
    potionRequirements[Potion::fireResistance->getId()] = "0 & 1 & !2 & !3 & 0+6";
    potionRequirements[Potion::heal->getId()] = "0 & !1 & 2 & !3";
    potionRequirements[Potion::poison->getId()] = "!0 & !1 & 2 & !3 & 2+6";
    potionRequirements[Potion::weakness->getId()] = "!0 & !1 & !2 & 3 & 3+6";
    potionRequirements[Potion::harm->getId()] = "!0 & !1 & 2 & 3";
    potionRequirements[Potion::moveSlowdown->getId()] = "!0 & 1 & !2 & 3 & 3+6";
    potionRequirements[Potion::damageBoost->getId()] = "0 & !1 & !2 & 3 & 3+6";

    potionAmplifiers[Potion::moveSpeed->getId()] = "5";
    potionAmplifiers[Potion::digSpeed->getId()] = "5";
    potionAmplifiers[Potion::damageBoost->getId()] = "5";
    potionAmplifiers[Potion::regeneration->getId()] = "5";
    potionAmplifiers[Potion::harm->getId()] = "5";
    potionAmplifiers[Potion::heal->getId()] = "5";
    potionAmplifiers[Potion::resistance->getId()] = "5";
    potionAmplifiers[Potion::poison->getId()] = "5";
    potionHelperInitialized = true;
}

bool PotionHelper::checkFlag(int value, int bit)
{
    return (value & JavaArithmetic::intShl(1, bit)) != 0;
}

int PotionHelper::isFlagSet(int value, int bit)
{
    return checkFlag(value, bit) ? 1 : 0;
}

int PotionHelper::isFlagUnset(int value, int bit)
{
    return checkFlag(value, bit) ? 0 : 1;
}

int PotionHelper::func_40352_a(int value)
{
    return func_40351_a(value, 5, 4, 3, 2, 1);
}

int PotionHelper::func_40354_a(const std::vector<PotionEffect> &effects)
{
    if (effects.empty())
        return 3694022;

    Potion::initPotions();
    float red = 0.0f;
    float green = 0.0f;
    float blue = 0.0f;
    float weight = 0.0f;

    for (const PotionEffect &effect : effects)
    {
        Potion *potion = Potion::getPotion(effect.getPotionID());
        if (potion == nullptr)
            continue;
        const int color = potion->getLiquidColor();
        for (int i = 0; i <= effect.getAmplifier(); ++i)
        {
            red += static_cast<float>((color >> 16) & 255) / 255.0f;
            green += static_cast<float>((color >> 8) & 255) / 255.0f;
            blue += static_cast<float>(color & 255) / 255.0f;
            weight += 1.0f;
        }
    }

    if (weight <= 0.0f)
        return 3694022;

    red = red / weight * 255.0f;
    green = green / weight * 255.0f;
    blue = blue / weight * 255.0f;
    return (static_cast<int>(red) << 16) | (static_cast<int>(green) << 8) | static_cast<int>(blue);
}

int PotionHelper::func_40358_a(int value, bool includeUnusable)
{
    ensureInitialized();
    if (!includeUnusable)
    {
        auto it = potionColorCache.find(value);
        if (it != potionColorCache.end())
            return it->second;
        const int color = func_40354_a(getPotionEffects(value, false));
        potionColorCache[value] = color;
        return color;
    }
    return func_40354_a(getPotionEffects(value, true));
}

std::string PotionHelper::func_40359_b(int value)
{
    const int index = func_40352_a(value);
    return index >= 0 && index < 32 ? potionPrefixes[index] : potionPrefixes[0];
}

int PotionHelper::func_40347_a(bool invert, bool multiply, bool negative, int comparison, int bit, int multiplier, int value)
{
    int result = 0;
    if (invert)
    {
        result = isFlagUnset(value, bit);
    }
    else if (comparison != -1)
    {
        if (comparison == 0 && countSetFlags(value) == bit)
            result = 1;
        else if (comparison == 1 && countSetFlags(value) > bit)
            result = 1;
        else if (comparison == 2 && countSetFlags(value) < bit)
            result = 1;
    }
    else
    {
        result = isFlagSet(value, bit);
    }

    if (multiply)
        result *= multiplier;
    if (negative)
        result *= -1;
    return result;
}

int PotionHelper::countSetFlags(int value)
{
    int count = 0;
    while (value > 0)
    {
        value &= value - 1;
        ++count;
    }
    return count;
}

int PotionHelper::func_40355_a(const std::string &expression, int start, int end, int value)
{
    if (start >= static_cast<int>(expression.size()) || end < 0 || start >= end)
        return 0;

    const std::size_t pipePos = expression.find('|', static_cast<std::size_t>(start));
    if (pipePos != std::string::npos && static_cast<int>(pipePos) < end)
    {
        const int left = func_40355_a(expression, start, static_cast<int>(pipePos) - 1, value);
        if (left > 0)
            return left;
        const int right = func_40355_a(expression, static_cast<int>(pipePos) + 1, end, value);
        return right > 0 ? right : 0;
    }

    const std::size_t ampPos = expression.find('&', static_cast<std::size_t>(start));
    if (ampPos != std::string::npos && static_cast<int>(ampPos) < end)
    {
        const int left = func_40355_a(expression, start, static_cast<int>(ampPos) - 1, value);
        if (left <= 0)
            return 0;
        const int right = func_40355_a(expression, static_cast<int>(ampPos) + 1, end, value);
        return right <= 0 ? 0 : (left > right ? left : right);
    }

    bool star = false;
    bool hasMultiplier = false;
    bool hasNumber = false;
    bool invert = false;
    bool negative = false;
    int comparison = -1;
    int bit = 0;
    int multiplier = 0;
    int result = 0;

    for (int i = start; i < end; ++i)
    {
        const char ch = expression[static_cast<std::size_t>(i)];
        if (ch >= '0' && ch <= '9')
        {
            if (star)
            {
                multiplier = ch - '0';
                hasMultiplier = true;
            }
            else
            {
                bit *= 10;
                bit += ch - '0';
                hasNumber = true;
            }
        }
        else if (ch == '*')
        {
            star = true;
        }
        else if (ch == '!')
        {
            if (hasNumber)
            {
                result += func_40347_a(invert, hasMultiplier, negative, comparison, bit, multiplier, value);
                star = false;
                hasMultiplier = false;
                hasNumber = false;
                invert = false;
                negative = false;
                multiplier = 0;
                bit = 0;
                comparison = -1;
            }
            invert = true;
        }
        else if (ch == '-')
        {
            if (hasNumber)
            {
                result += func_40347_a(invert, hasMultiplier, negative, comparison, bit, multiplier, value);
                star = false;
                hasMultiplier = false;
                hasNumber = false;
                invert = false;
                multiplier = 0;
                bit = 0;
                comparison = -1;
            }
            negative = true;
        }
        else if (ch == '=')
        {
            if (hasNumber)
            {
                result += func_40347_a(invert, hasMultiplier, negative, comparison, bit, multiplier, value);
                star = false;
                hasMultiplier = false;
                hasNumber = false;
                invert = false;
                negative = false;
                multiplier = 0;
                bit = 0;
            }
            comparison = 0;
        }
        else if (ch == '<')
        {
            if (hasNumber)
            {
                result += func_40347_a(invert, hasMultiplier, negative, comparison, bit, multiplier, value);
                star = false;
                hasMultiplier = false;
                hasNumber = false;
                invert = false;
                negative = false;
                multiplier = 0;
                bit = 0;
            }
            comparison = 2;
        }
        else if (ch == '>')
        {
            if (hasNumber)
            {
                result += func_40347_a(invert, hasMultiplier, negative, comparison, bit, multiplier, value);
                star = false;
                hasMultiplier = false;
                hasNumber = false;
                invert = false;
                negative = false;
                multiplier = 0;
                bit = 0;
            }
            comparison = 1;
        }
        else if (ch == '+' && hasNumber)
        {
            result += func_40347_a(invert, hasMultiplier, negative, comparison, bit, multiplier, value);
            star = false;
            hasMultiplier = false;
            hasNumber = false;
            invert = false;
            negative = false;
            multiplier = 0;
            bit = 0;
            comparison = -1;
        }
    }

    if (hasNumber)
        result += func_40347_a(invert, hasMultiplier, negative, comparison, bit, multiplier, value);
    return result;
}

std::vector<PotionEffect> PotionHelper::getPotionEffects(int value, bool includeUnusable)
{
    ensureInitialized();
    std::vector<PotionEffect> effects;
    for (int id = 0; id < Potion::POTION_TYPE_COUNT; ++id)
    {
        Potion *potion = Potion::potionTypes[id];
        if (potion == nullptr || (potion->isUsable() && !includeUnusable))
            continue;

        auto requirement = potionRequirements.find(potion->getId());
        if (requirement == potionRequirements.end())
            continue;

        int duration = func_40355_a(requirement->second, 0, static_cast<int>(requirement->second.size()), value);
        if (duration <= 0)
            continue;

        int amplifier = 0;
        auto amplifierExpression = potionAmplifiers.find(potion->getId());
        if (amplifierExpression != potionAmplifiers.end())
        {
            amplifier = func_40355_a(amplifierExpression->second, 0, static_cast<int>(amplifierExpression->second.size()), value);
            if (amplifier < 0)
                amplifier = 0;
        }

        if (potion->isInstant())
        {
            duration = 1;
        }
        else
        {
            duration = 1200 * (duration * 3 + (duration - 1) * 2);
            duration = JavaArithmetic::intShr(duration, amplifier);
            duration = JavaArithmetic::longToInt(JavaArithmetic::roundDouble(static_cast<double>(duration) * potion->getEffectiveness()));
            if ((value & 16384) != 0)
                duration = JavaArithmetic::longToInt(JavaArithmetic::roundDouble(static_cast<double>(duration) * 0.75 + 0.5));
        }
        effects.emplace_back(potion->getId(), duration, amplifier);
    }
    return effects;
}

int PotionHelper::brewBitOperations(int value, int bit, bool clear, bool toggle, bool requireSet)
{
    if (requireSet)
    {
        if (!checkFlag(value, bit))
            return 0;
    }
    else if (clear)
    {
        value &= ~JavaArithmetic::intShl(1, bit);
    }
    else if (toggle)
    {
        if ((value & JavaArithmetic::intShl(1, bit)) != 0)
            value &= ~JavaArithmetic::intShl(1, bit);
        else
            value |= JavaArithmetic::intShl(1, bit);
    }
    else
    {
        value |= JavaArithmetic::intShl(1, bit);
    }
    return value;
}

int PotionHelper::applyIngredient(int value, const std::string &ingredient)
{
    bool hasNumber = false;
    bool toggle = false;
    bool clear = false;
    bool requireSet = false;
    int bit = 0;

    for (char ch : ingredient)
    {
        if (ch >= '0' && ch <= '9')
        {
            bit *= 10;
            bit += ch - '0';
            hasNumber = true;
        }
        else if (ch == '!')
        {
            if (hasNumber)
            {
                value = brewBitOperations(value, bit, clear, toggle, requireSet);
                requireSet = false;
                clear = false;
                hasNumber = false;
                bit = 0;
            }
            toggle = true;
        }
        else if (ch == '-')
        {
            if (hasNumber)
            {
                value = brewBitOperations(value, bit, clear, toggle, requireSet);
                requireSet = false;
                toggle = false;
                hasNumber = false;
                bit = 0;
            }
            clear = true;
        }
        else if (ch == '+')
        {
            if (hasNumber)
            {
                value = brewBitOperations(value, bit, clear, toggle, requireSet);
                requireSet = false;
                toggle = false;
                clear = false;
                hasNumber = false;
                bit = 0;
            }
        }
        else if (ch == '&')
        {
            if (hasNumber)
            {
                value = brewBitOperations(value, bit, clear, toggle, requireSet);
                requireSet = false;
                toggle = false;
                clear = false;
                hasNumber = false;
                bit = 0;
            }
            requireSet = true;
        }
    }

    if (hasNumber)
        value = brewBitOperations(value, bit, clear, toggle, requireSet);
    return value & 32767;
}

int PotionHelper::func_40351_a(int value, int bit1, int bit2, int bit3, int bit4, int bit5)
{
    return (checkFlag(value, bit1) ? 16 : 0) |
           (checkFlag(value, bit2) ? 8 : 0) |
           (checkFlag(value, bit3) ? 4 : 0) |
           (checkFlag(value, bit4) ? 2 : 0) |
           (checkFlag(value, bit5) ? 1 : 0);
}
