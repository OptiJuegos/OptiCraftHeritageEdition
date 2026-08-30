#include "platform/Log.h"
#include "AchievementList.h"

#include "Achievement.h"
#include "Item.h"
#include "Block.h"
#include <cstdio>

int_t AchievementList::minDisplayColumn = 0;
int_t AchievementList::minDisplayRow = 0;
int_t AchievementList::maxDisplayColumn = 0;
int_t AchievementList::maxDisplayRow = 0;

std::vector<Achievement*> AchievementList::achievementList;

Achievement *AchievementList::openInventory = nullptr;
Achievement *AchievementList::mineWood = nullptr;
Achievement *AchievementList::buildWorkBench = nullptr;
Achievement *AchievementList::buildPickaxe = nullptr;
Achievement *AchievementList::buildFurnace = nullptr;
Achievement *AchievementList::acquireIron = nullptr;
Achievement *AchievementList::buildHoe = nullptr;
Achievement *AchievementList::makeBread = nullptr;
Achievement *AchievementList::bakeCake = nullptr;
Achievement *AchievementList::buildBetterPickaxe = nullptr;
Achievement *AchievementList::cookFish = nullptr;
Achievement *AchievementList::onARail = nullptr;
Achievement *AchievementList::buildSword = nullptr;
Achievement *AchievementList::killEnemy = nullptr;
Achievement *AchievementList::killCow = nullptr;
Achievement *AchievementList::flyPig = nullptr;
Achievement *AchievementList::snipeSkeleton = nullptr;
Achievement *AchievementList::diamonds = nullptr;
Achievement *AchievementList::portal = nullptr;
Achievement *AchievementList::ghast = nullptr;
Achievement *AchievementList::blazeRod = nullptr;
Achievement *AchievementList::potion = nullptr;
Achievement *AchievementList::theEnd = nullptr;
Achievement *AchievementList::theEnd2 = nullptr;
Achievement *AchievementList::enchantments = nullptr;
Achievement *AchievementList::overkill = nullptr;
Achievement *AchievementList::bookcase = nullptr;

void AchievementList::initialize()
{
    openInventory        = (new Achievement(0,  "openInventory",      0,  0,  Item::book,             nullptr))->setIndependent()->registerAchievement();
    mineWood             = (new Achievement(1,  "mineWood",           2,  1,  Block::wood,             openInventory))->registerAchievement();
    buildWorkBench       = (new Achievement(2,  "buildWorkBench",     4, -1,  Block::workbench,        mineWood))->registerAchievement();
    buildPickaxe         = (new Achievement(3,  "buildPickaxe",       4,  2,  Item::pickaxeWood,       buildWorkBench))->registerAchievement();
    buildFurnace         = (new Achievement(4,  "buildFurnace",       3,  4,  Block::stoneOvenActive,  buildPickaxe))->registerAchievement();
    acquireIron          = (new Achievement(5,  "acquireIron",        1,  4,  Item::ingotIron,         buildFurnace))->registerAchievement();
    buildHoe             = (new Achievement(6,  "buildHoe",           2, -3,  Item::hoeWood,           buildWorkBench))->registerAchievement();
    makeBread            = (new Achievement(7,  "makeBread",         -1, -3,  Item::bread,             buildHoe))->registerAchievement();
    bakeCake             = (new Achievement(8,  "bakeCake",           0, -5,  Item::cake,              buildHoe))->registerAchievement();
    buildBetterPickaxe   = (new Achievement(9,  "buildBetterPickaxe", 6,  2,  Item::pickaxeStone,      buildPickaxe))->registerAchievement();
    cookFish             = (new Achievement(10, "cookFish",           2,  6,  Item::fishCooked,        buildFurnace))->registerAchievement();
    onARail              = (new Achievement(11, "onARail",            2,  3,  Block::rail,             acquireIron))->setSpecial()->registerAchievement();
    buildSword           = (new Achievement(12, "buildSword",         6, -1,  Item::swordWood,         buildWorkBench))->registerAchievement();
    killEnemy            = (new Achievement(13, "killEnemy",          8, -1,  Item::bone,              buildSword))->registerAchievement();
    killCow              = (new Achievement(14, "killCow",            7, -3,  Item::leather,           buildSword))->registerAchievement();
    flyPig               = (new Achievement(15, "flyPig",             8, -4,  Item::saddle,            killCow))->setSpecial()->registerAchievement();
    snipeSkeleton        = (new Achievement(16, "snipeSkeleton",       7,  0,  Item::bow,               killEnemy))->setSpecial()->registerAchievement();
    diamonds             = (new Achievement(17, "diamonds",            -1,  5,  Item::diamond,           acquireIron))->registerAchievement();
    portal               = (new Achievement(18, "portal",              -1,  7,  Block::obsidian,         diamonds))->registerAchievement();
    ghast                = (new Achievement(19, "ghast",               -4,  8,  Item::ghastTear,         portal))->setSpecial()->registerAchievement();
    blazeRod             = (new Achievement(20, "blazeRod",             0,  9,  Item::blazeRod,          portal))->registerAchievement();
    potion               = (new Achievement(21, "potion",               2,  8,  Item::potion,            blazeRod))->registerAchievement();
    theEnd               = (new Achievement(22, "theEnd",               3, 10,  Item::eyeOfEnder,        blazeRod))->setSpecial()->registerAchievement();
    theEnd2              = (new Achievement(23, "theEnd2",              4, 13,  Block::dragonEgg,        theEnd))->setSpecial()->registerAchievement();
    enchantments         = (new Achievement(24, "enchantments",        -4,  4,  Block::enchantmentTable, diamonds))->registerAchievement();
    overkill             = (new Achievement(25, "overkill",            -4,  1,  Item::swordDiamond,      enchantments))->setSpecial()->registerAchievement();
    bookcase             = (new Achievement(26, "bookcase",            -3,  6,  Block::bookShelf,        enchantments))->registerAchievement();

    MC_LOG_INFO("game", "%d achievements\n", (int)achievementList.size());
}

void AchievementList::cleanup()
{
    for (Achievement *ach : achievementList)
    {
        delete ach;
    }
    achievementList.clear();

    openInventory = nullptr;
    mineWood = nullptr;
    buildWorkBench = nullptr;
    buildPickaxe = nullptr;
    buildFurnace = nullptr;
    acquireIron = nullptr;
    buildHoe = nullptr;
    makeBread = nullptr;
    bakeCake = nullptr;
    buildBetterPickaxe = nullptr;
    cookFish = nullptr;
    onARail = nullptr;
    buildSword = nullptr;
    killEnemy = nullptr;
    killCow = nullptr;
    flyPig = nullptr;
    snipeSkeleton = nullptr;
    diamonds = nullptr;
    portal = nullptr;
    ghast = nullptr;
    blazeRod = nullptr;
    potion = nullptr;
    theEnd = nullptr;
    theEnd2 = nullptr;
    enchantments = nullptr;
    overkill = nullptr;
    bookcase = nullptr;
}

