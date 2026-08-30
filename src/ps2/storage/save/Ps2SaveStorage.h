#pragma once

#ifdef PS2_PLATFORM

#include <string>

namespace Ps2SaveStorage
{
    enum class Target
    {
        Disabled,
        MemoryCard,
        MassStorage
    };

    void setTarget(Target target);
    Target target();
    bool enabled();
    std::string root();
    const char* displayRoot();
}

#endif // PS2_PLATFORM
