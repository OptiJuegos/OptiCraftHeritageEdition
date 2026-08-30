#pragma once
#include <string>
#include <vector>
#include "java/Random.h"
class EnchantmentNameParts
{
public:
    static EnchantmentNameParts &getInstance();
    std::string generateRandomEnchantName();
    void setRandSeed(long_t seed);
private:
    EnchantmentNameParts();
    Random rand;
    std::vector<std::string> wordList;
};
