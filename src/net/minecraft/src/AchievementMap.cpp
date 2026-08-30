#include "AchievementMap.h"
#include "GameResources.h"
#include "java/String.h"

#include <fstream>
#include <sstream>
#include <cstdio>
#include <limits>

AchievementMap& AchievementMap::getInstance()
{
    static AchievementMap instance;
    return instance;
}

AchievementMap::AchievementMap()
{
    try
    {
        auto file = GameResources::open("/achievement/map.txt");
        if (!file || !file->is_open())
            return;

        std::string line;
        while (std::getline(*file, line))
        {
            if (line.empty())
                continue;
            const std::vector<jstring> parts = String::splitJava(line, ',');
            if (parts.size() < 2)
                continue;
            long_t parsed = 0;
            if (!String::tryParseLong(parts[0], parsed) ||
                parsed < std::numeric_limits<int_t>::min() ||
                parsed > std::numeric_limits<int_t>::max())
                continue;
            const int_t id = static_cast<int_t>(parsed);
            guidMap[id] = parts[1];
        }
    }
    catch (...)
    {
        // silently ignore — matching Java catch(Exception exception) { e.printStackTrace() }
    }
}

std::string AchievementMap::getGuid(int_t i)
{
    AchievementMap &instance = getInstance();
    auto it = instance.guidMap.find(i);
    if (it != instance.guidMap.end())
        return it->second;
    return "";
}
