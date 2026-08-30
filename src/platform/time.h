#pragma once

#include <chrono>

inline float getTimeS()
{
    using namespace std::chrono;
    static const steady_clock::time_point start = steady_clock::now();
    return duration_cast<duration<float>>(steady_clock::now() - start).count();
}

inline unsigned long long getTimeUS()
{
    using namespace std::chrono;
    static const steady_clock::time_point start = steady_clock::now();
    return (unsigned long long)duration_cast<microseconds>(steady_clock::now() - start).count();
}
