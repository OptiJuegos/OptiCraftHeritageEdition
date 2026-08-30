#pragma once
#ifdef WII_PLATFORM

namespace WiiConsole
{
void write(const char* fmt, ...)
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((format(printf, 1, 2)))
#endif
    ;
}

#endif // WII_PLATFORM
