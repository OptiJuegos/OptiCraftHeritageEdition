#include "LocalStatsFormat.h"

#include <cctype>
#include <limits>

namespace
{
bool parseInteger(const std::string &text, std::size_t &position, int_t &value)
{
    while (position < text.size() && std::isspace(static_cast<unsigned char>(text[position])))
        ++position;
    if (position >= text.size())
        return false;

    bool negative = false;
    if (text[position] == '+' || text[position] == '-')
    {
        negative = text[position] == '-';
        ++position;
    }
    if (position >= text.size() || text[position] < '0' || text[position] > '9')
        return false;

    const unsigned long long limit = negative
        ? static_cast<unsigned long long>(-(std::numeric_limits<int_t>::min() + 1)) + 1ULL
        : static_cast<unsigned long long>(std::numeric_limits<int_t>::max());
    unsigned long long magnitude = 0;
    while (position < text.size() && text[position] >= '0' && text[position] <= '9')
    {
        const unsigned int digit = static_cast<unsigned int>(text[position] - '0');
        if (magnitude > (limit - digit) / 10ULL)
            return false;
        magnitude = magnitude * 10ULL + digit;
        ++position;
    }

    if (negative && magnitude == limit)
        value = std::numeric_limits<int_t>::min();
    else
        value = negative ? -static_cast<int_t>(magnitude) : static_cast<int_t>(magnitude);
    return true;
}
}

namespace LocalStatsFormat
{
void parse(const std::string &text, std::vector<Entry> &entries)
{
    entries.clear();
    std::size_t position = 0;
    while (position < text.size())
    {
        int_t statId = 0;
        int_t value = 0;
        if (!parseInteger(text, position, statId) || !parseInteger(text, position, value))
            break;
        entries.emplace_back(statId, value);
    }
}

std::string serialize(const std::vector<Entry> &entries)
{
    std::string text;
    for (const Entry &entry : entries)
    {
        text += std::to_string(entry.first);
        text += ' ';
        text += std::to_string(entry.second);
        text += '\n';
    }
    return text;
}
}
