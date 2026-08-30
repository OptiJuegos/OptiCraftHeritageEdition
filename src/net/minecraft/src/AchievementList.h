#pragma once

#include <vector>

#include "Achievement.h"
#include "java/Type.h"

// net.minecraft.src.AchievementList
class AchievementList
{
public:
    static void initialize();
    static void cleanup();

    static int_t minDisplayColumn;
    static int_t minDisplayRow;
    static int_t maxDisplayColumn;
    static int_t maxDisplayRow;

    static std::vector<Achievement*> achievementList;

    static Achievement *openInventory;
    static Achievement *mineWood;
    static Achievement *buildWorkBench;
    static Achievement *buildPickaxe;
    static Achievement *buildFurnace;
    static Achievement *acquireIron;
    static Achievement *buildHoe;
    static Achievement *makeBread;
    static Achievement *bakeCake;
    static Achievement *buildBetterPickaxe;
    static Achievement *cookFish;
    static Achievement *onARail;
    static Achievement *buildSword;
    static Achievement *killEnemy;
    static Achievement *killCow;
    static Achievement *flyPig;
    static Achievement *snipeSkeleton;
    static Achievement *diamonds;
    static Achievement *portal;
    static Achievement *ghast;
    static Achievement *blazeRod;
    static Achievement *potion;
    static Achievement *theEnd;
    static Achievement *theEnd2;
    static Achievement *enchantments;
    static Achievement *overkill;
    static Achievement *bookcase;
};
