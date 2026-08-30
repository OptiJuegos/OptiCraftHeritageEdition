#pragma once

#include <string>
#include <map>
#include "java/Type.h"

// net.minecraft.src.AchievementMap
class AchievementMap
{
private:
    AchievementMap();

public:
    static std::string getGuid(int_t i);

private:
    static AchievementMap& getInstance();
    std::map<int_t, std::string> guidMap;
};
