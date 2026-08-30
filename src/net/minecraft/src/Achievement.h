#pragma once

#include "StatBase.h"
#include <string>

class Item;
class Block;
class ItemStack;
class IStatStringFormat;

// net.minecraft.src.Achievement
class Achievement : public StatBase
{
public:
    Achievement(int_t i, const std::string &s, int_t j, int_t k, Item *item, Achievement *achievement);
    Achievement(int_t i, const std::string &s, int_t j, int_t k, Block *block, Achievement *achievement);
    Achievement(int_t i, const std::string &s, int_t j, int_t k, ItemStack *itemstack, Achievement *achievement);

    Achievement* setIndependent() override;
    Achievement* setSpecial();
    Achievement* registerAchievement();

    bool isAchievement() override;
    std::string getDescription();
    Achievement* setStatStringFormatter(IStatStringFormat *formatter);
    bool getSpecial();
    StatBase* registerStat() override;
    StatBase* setIndependentStat();

    const int_t displayColumn;
    const int_t displayRow;
    Achievement *parentAchievement;
    ItemStack *theItemStack;

private:
    std::string achievementDescription;
    IStatStringFormat *statStringFormatter;
    bool isSpecialAchievement;
};
