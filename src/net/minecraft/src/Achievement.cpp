#include "Achievement.h"

#include "Item.h"
#include "Block.h"
#include "ItemStack.h"
#include "StatCollector.h"
#include "IStatStringFormat.h"
#include "AchievementList.h"

Achievement::Achievement(int_t i, const std::string &s, int_t j, int_t k, Item *item, Achievement *achievement) :
    Achievement(i, s, j, k, new ItemStack(item), achievement)
{
}

Achievement::Achievement(int_t i, const std::string &s, int_t j, int_t k, Block *block, Achievement *achievement) :
    Achievement(i, s, j, k, new ItemStack(block), achievement)
{
}

Achievement::Achievement(int_t i, const std::string &s, int_t j, int_t k, ItemStack *itemstack, Achievement *achievement) :
    StatBase(0x500000 + i, StatCollector::translateToLocal("achievement." + s)),
    displayColumn(j),
    displayRow(k),
    parentAchievement(achievement),
    theItemStack(itemstack),
    statStringFormatter(nullptr),
    isSpecialAchievement(false)
{
    achievementDescription = StatCollector::translateToLocal("achievement." + s + ".desc");

    if (j < AchievementList::minDisplayColumn)
        AchievementList::minDisplayColumn = j;
    if (k < AchievementList::minDisplayRow)
        AchievementList::minDisplayRow = k;
    if (j > AchievementList::maxDisplayColumn)
        AchievementList::maxDisplayColumn = j;
    if (k > AchievementList::maxDisplayRow)
        AchievementList::maxDisplayRow = k;
}

Achievement* Achievement::setIndependent()
{
    independent = true;
    return this;
}

Achievement* Achievement::setSpecial()
{
    isSpecialAchievement = true;
    return this;
}

Achievement* Achievement::registerAchievement()
{
    StatBase::registerStat();
    AchievementList::achievementList.push_back(this);
    return this;
}

bool Achievement::isAchievement()
{
    return true;
}

std::string Achievement::getDescription()
{
    if (statStringFormatter != nullptr)
        return statStringFormatter->formatString(achievementDescription);
    return achievementDescription;
}

Achievement* Achievement::setStatStringFormatter(IStatStringFormat *formatter)
{
    if (statStringFormatter != formatter)
    {
        delete statStringFormatter;
        statStringFormatter = formatter;
    }
    return this;
}

bool Achievement::getSpecial()
{
    return isSpecialAchievement;
}

StatBase* Achievement::registerStat()
{
    return registerAchievement();
}

StatBase* Achievement::setIndependentStat()
{
    return setIndependent();
}
