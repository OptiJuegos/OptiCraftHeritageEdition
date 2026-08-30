#include "EnchantmentHelper.h"

#include "java/Arithmetic.h"
#include <algorithm>
#include <cstdint>

#include "DamageSource.h"
#include "Enchantment.h"
#include "EnchantmentModifierDamage.h"
#include "EnchantmentModifierLiving.h"
#include "EnumEnchantmentType.h"
#include "InventoryPlayer.h"
#include "Item.h"
#include "ItemStack.h"
#include "NBTTagCompound.h"
#include "NBTTagList.h"
#include "WeightedRandom.h"
#include "java/Random.h"

namespace
{
Random gEnchantmentRand;

void applyEnchantmentModifier(IEnchantmentModifier &modifier, ItemStack *stack)
{
    if (stack == nullptr)
        return;
    NBTTagList *list = stack->getEnchantmentTagList();
    if (list == nullptr)
        return;
    for (int i = 0; i < list->tagCount(); ++i)
    {
        NBTTagCompound *tag = dynamic_cast<NBTTagCompound *>(list->tagAt(i));
        if (tag == nullptr)
            continue;
        const int id = tag->getShort("id");
        const int level = tag->getShort("lvl");
        if (id >= 0 && id < 256 && Enchantment::enchantmentsList[id] != nullptr)
            modifier.calculateModifier(Enchantment::enchantmentsList[id], level);
    }
}


std::uint32_t javaHashMapHash(std::uint32_t hash)
{
    hash ^= (hash >> 20) ^ (hash >> 12);
    return hash ^ (hash >> 7) ^ (hash >> 4);
}

void reorderLikeJavaIntegerHashMap(std::vector<EnchantmentData> &data)
{
    if (data.size() <= 1)
        return;

    // Minecraft 1.2.5 used java.util.HashMap with the default capacity/load
    // factor. Iteration walks table buckets from low to high and each bucket's
    // linked list from head to tail. New entries are inserted at the head; the
    // JDK 6 transfer routine also reverses chains while resizing. Enchantment
    // selection consumes Random through WeightedRandom over map.values(), so
    // preserving this order is required for deterministic Java-equivalent
    // enchantment results.
    std::size_t capacity = 16;
    std::size_t threshold = 12;
    std::size_t size = 0;
    std::vector<std::vector<std::size_t>> buckets(capacity);

    auto transfer = [&](std::size_t newCapacity)
    {
        std::vector<std::vector<std::size_t>> next(newCapacity);
        for (const auto &bucket : buckets)
        {
            for (std::size_t dataIndex : bucket)
            {
                const std::uint32_t key = static_cast<std::uint32_t>(data[dataIndex].enchantmentobj->effectId);
                const std::size_t bucketIndex = javaHashMapHash(key) & (newCapacity - 1u);
                next[bucketIndex].insert(next[bucketIndex].begin(), dataIndex);
            }
        }
        buckets.swap(next);
        capacity = newCapacity;
        threshold = capacity * 3u / 4u;
    };

    for (std::size_t dataIndex = 0; dataIndex < data.size(); ++dataIndex)
    {
        const std::uint32_t key = static_cast<std::uint32_t>(data[dataIndex].enchantmentobj->effectId);
        const std::size_t bucketIndex = javaHashMapHash(key) & (capacity - 1u);
        buckets[bucketIndex].insert(buckets[bucketIndex].begin(), dataIndex);
        const std::size_t previousSize = size++;
        if (previousSize >= threshold)
            transfer(capacity * 2u);
    }

    std::vector<EnchantmentData> ordered;
    ordered.reserve(data.size());
    for (const auto &bucket : buckets)
    {
        for (std::size_t dataIndex : bucket)
            ordered.push_back(data[dataIndex]);
    }
    data.swap(ordered);
}

EnchantmentData *chooseWeighted(Random &random, std::vector<EnchantmentData> &data)
{
    std::vector<WeightedRandomChoice *> choices;
    choices.reserve(data.size());
    for (EnchantmentData &entry : data)
        choices.push_back(&entry);
    return static_cast<EnchantmentData *>(WeightedRandom::getRandomItem(random, choices));
}
}

int EnchantmentHelper::getEnchantmentLevel(int effectId, ItemStack *stack)
{
    if (stack == nullptr)
        return 0;
    NBTTagList *list = stack->getEnchantmentTagList();
    if (list == nullptr)
        return 0;
    for (int i = 0; i < list->tagCount(); ++i)
    {
        NBTTagCompound *tag = dynamic_cast<NBTTagCompound *>(list->tagAt(i));
        if (tag != nullptr && tag->getShort("id") == effectId)
            return tag->getShort("lvl");
    }
    return 0;
}

int EnchantmentHelper::getMaxEnchantmentLevel(int effectId, ItemStack **stacks, int count)
{
    int maximum = 0;
    for (int i = 0; i < count; ++i)
        maximum = std::max(maximum, getEnchantmentLevel(effectId, stacks[i]));
    return maximum;
}

int EnchantmentHelper::getEnchantmentModifierDamage(InventoryPlayer *inventory, const DamageSource &source)
{
    if (inventory == nullptr)
        return 0;
    EnchantmentModifierDamage modifier;
    modifier.damageSource = &source;
    for (int i = 0; i < 4; ++i)
        applyEnchantmentModifier(modifier, inventory->armorInventory[i]);
    modifier.damageModifier = std::min(modifier.damageModifier, 25);
    return ((modifier.damageModifier + 1) >> 1) + gEnchantmentRand.nextInt((modifier.damageModifier >> 1) + 1);
}

int EnchantmentHelper::getEnchantmentModifierLiving(InventoryPlayer *inventory, EntityLiving *living)
{
    if (inventory == nullptr)
        return 0;
    EnchantmentModifierLiving modifier;
    modifier.entityLiving = living;
    applyEnchantmentModifier(modifier, inventory->getCurrentItem());
    return modifier.livingModifier > 0 ? 1 + gEnchantmentRand.nextInt(modifier.livingModifier) : 0;
}

int EnchantmentHelper::getKnockbackModifier(InventoryPlayer *inventory, EntityLiving *) { return inventory != nullptr && Enchantment::knockback != nullptr ? getEnchantmentLevel(Enchantment::knockback->effectId, inventory->getCurrentItem()) : 0; }
int EnchantmentHelper::getFireAspectModifier(InventoryPlayer *inventory, EntityLiving *) { return inventory != nullptr && Enchantment::fireAspect != nullptr ? getEnchantmentLevel(Enchantment::fireAspect->effectId, inventory->getCurrentItem()) : 0; }
int EnchantmentHelper::getRespiration(InventoryPlayer *inventory) { return inventory != nullptr && Enchantment::respiration != nullptr ? getMaxEnchantmentLevel(Enchantment::respiration->effectId, inventory->armorInventory, 4) : 0; }
int EnchantmentHelper::getEfficiencyModifier(InventoryPlayer *inventory) { return inventory != nullptr && Enchantment::efficiency != nullptr ? getEnchantmentLevel(Enchantment::efficiency->effectId, inventory->getCurrentItem()) : 0; }
int EnchantmentHelper::getUnbreakingModifier(InventoryPlayer *inventory) { return inventory != nullptr && Enchantment::unbreaking != nullptr ? getEnchantmentLevel(Enchantment::unbreaking->effectId, inventory->getCurrentItem()) : 0; }
bool EnchantmentHelper::getSilkTouchModifier(InventoryPlayer *inventory) { return inventory != nullptr && Enchantment::silkTouch != nullptr && getEnchantmentLevel(Enchantment::silkTouch->effectId, inventory->getCurrentItem()) > 0; }
int EnchantmentHelper::getFortuneModifier(InventoryPlayer *inventory) { return inventory != nullptr && Enchantment::fortune != nullptr ? getEnchantmentLevel(Enchantment::fortune->effectId, inventory->getCurrentItem()) : 0; }
int EnchantmentHelper::getLootingModifier(InventoryPlayer *inventory) { return inventory != nullptr && Enchantment::looting != nullptr ? getEnchantmentLevel(Enchantment::looting->effectId, inventory->getCurrentItem()) : 0; }
bool EnchantmentHelper::getAquaAffinityModifier(InventoryPlayer *inventory) { return inventory != nullptr && Enchantment::aquaAffinity != nullptr && getMaxEnchantmentLevel(Enchantment::aquaAffinity->effectId, inventory->armorInventory, 4) > 0; }

int EnchantmentHelper::calcItemStackEnchantability(Random &random, int slot, int bookshelves, ItemStack *stack)
{
    if (stack == nullptr || stack->getItem() == nullptr)
        return 0;
    const int itemEnchantability = stack->getItem()->getItemEnchantability();
    if (itemEnchantability <= 0)
        return 0;
    bookshelves = std::min(bookshelves, 30);
    bookshelves = 1 + (bookshelves >> 1) + random.nextInt(bookshelves + 1);
    const int value = random.nextInt(5) + bookshelves;
    if (slot == 0) return (value >> 1) + 1;
    if (slot == 1) return value * 2 / 3 + 1;
    return value;
}

void EnchantmentHelper::addRandomEnchantment(Random &random, ItemStack *stack, int level)
{
    std::vector<EnchantmentData> list = buildEnchantmentList(random, stack, level);
    for (const EnchantmentData &entry : list)
        stack->addEnchantment(entry.enchantmentobj, entry.enchantmentLevel);
}

std::vector<EnchantmentData> EnchantmentHelper::buildEnchantmentList(Random &random, ItemStack *stack, int level)
{
    std::vector<EnchantmentData> result;
    if (stack == nullptr || stack->getItem() == nullptr)
        return result;
    int itemEnchantability = stack->getItem()->getItemEnchantability();
    if (itemEnchantability <= 0)
        return result;

    itemEnchantability = 1 + random.nextInt((itemEnchantability >> 1) + 1) + random.nextInt((itemEnchantability >> 1) + 1);
    int adjusted = itemEnchantability + level;
    const float variance = (random.nextFloat() + random.nextFloat() - 1.0f) * 0.25f;
    adjusted = JavaArithmetic::floatToInt(static_cast<float>(adjusted) * (1.0f + variance) + 0.5f);

    std::vector<EnchantmentData> candidates = mapEnchantmentData(adjusted, stack);
    if (candidates.empty())
        return result;

    EnchantmentData *selected = chooseWeighted(random, candidates);
    if (selected == nullptr)
        return result;
    result.push_back(*selected);

    for (int threshold = adjusted >> 1; random.nextInt(50) <= threshold; threshold >>= 1)
    {
        candidates.erase(std::remove_if(candidates.begin(), candidates.end(), [&result](const EnchantmentData &candidate) {
            for (const EnchantmentData &existing : result)
            {
                if (!existing.enchantmentobj->canApplyTogether(candidate.enchantmentobj))
                    return true;
            }
            return false;
        }), candidates.end());
        if (!candidates.empty())
        {
            selected = chooseWeighted(random, candidates);
            if (selected != nullptr)
                result.push_back(*selected);
        }
    }
    return result;
}

std::vector<EnchantmentData> EnchantmentHelper::mapEnchantmentData(int enchantability, ItemStack *stack)
{
    std::vector<EnchantmentData> result;
    Item *item = stack != nullptr ? stack->getItem() : nullptr;
    if (item == nullptr)
        return result;
    for (Enchantment *enchantment : Enchantment::enchantmentsList)
    {
        if (enchantment == nullptr || !EnumEnchantmentTypeHelper::canEnchantItem(enchantment->type, item))
            continue;

        int selectedLevel = 0;
        for (int level = enchantment->getMinLevel(); level <= enchantment->getMaxLevel(); ++level)
        {
            if (enchantability >= enchantment->getMinEnchantability(level) && enchantability <= enchantment->getMaxEnchantability(level))
                selectedLevel = level;
        }
        if (selectedLevel > 0)
            result.emplace_back(enchantment, selectedLevel);
    }
    reorderLikeJavaIntegerHashMap(result);
    return result;
}
