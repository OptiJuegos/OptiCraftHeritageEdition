#pragma once

#include "java/Type.h"

#include <string>
#include <utility>
#include <vector>

namespace LocalStatsFormat
{
using Entry = std::pair<int_t, int_t>;

void parse(const std::string &text, std::vector<Entry> &entries);
std::string serialize(const std::vector<Entry> &entries);
}
