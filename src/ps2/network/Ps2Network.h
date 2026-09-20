#pragma once

#ifdef PS2_PLATFORM

#include <string>

namespace Ps2Network
{
bool initialize();
bool isReady();
std::string localAddress();
}

#endif
