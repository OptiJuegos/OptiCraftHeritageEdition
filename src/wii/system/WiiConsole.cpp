#ifdef WII_PLATFORM

#include "wii/system/WiiConsole.h"

#include <cstdarg>
#include <cstdio>

namespace WiiConsole
{

void write(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    std::vprintf(fmt, args);
    va_end(args);
}

} // namespace WiiConsole

#endif // WII_PLATFORM
