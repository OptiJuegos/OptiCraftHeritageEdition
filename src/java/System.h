#pragma once

#include "Type.h"
#include <string>

namespace System
{

long_t currentTimeMillis();
long_t nanoTime();
bool openURL(const std::string &url);

}
