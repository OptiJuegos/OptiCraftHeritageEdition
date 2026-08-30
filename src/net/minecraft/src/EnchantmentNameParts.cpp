#include "EnchantmentNameParts.h"
#include <sstream>
EnchantmentNameParts::EnchantmentNameParts()
{
    std::istringstream stream("the elder scrolls klaatu berata niktu xyzzy bless curse light darkness fire air earth water hot dry cold wet ignite snuff embiggen twist shorten stretch fiddle destroy imbue galvanize enchant free limited range of towards inside sphere cube self other ball mental physical grow shrink demon elemental spirit animal creature beast humanoid undead fresh stale");
    std::string word;
    while (stream >> word) wordList.push_back(word);
}
EnchantmentNameParts &EnchantmentNameParts::getInstance() { static EnchantmentNameParts instance; return instance; }
std::string EnchantmentNameParts::generateRandomEnchantName()
{
    const int count = rand.nextInt(2) + 3;
    std::string result;
    for (int i = 0; i < count; ++i)
    {
        if (i > 0) result += " ";
        result += wordList[rand.nextInt(static_cast<int>(wordList.size()))];
    }
    return result;
}
void EnchantmentNameParts::setRandSeed(long_t seed) { rand.setSeed(seed); }
