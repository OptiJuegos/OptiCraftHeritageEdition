#pragma once

#ifdef PS2_PLATFORM

#include <string>
#include <vector>
#include <cstddef>

namespace Ps2SaveStorage
{
    enum class Target
    {
        Disabled,
        MemoryCard,
        MassStorage
    };

    // Configuration never follows the world target.
    std::string configRoot();
    bool readConfiguration(const std::string &path, std::vector<unsigned char> &out);
    bool writeConfiguration(const std::string &path, const void *data, std::size_t length);
    bool available(Target target);
    bool configurationSaveFailed();
    void reportConfigurationSave(bool success);
    void setTarget(Target target);
    Target target();
    bool enabled();
    std::string root();
    const char* displayRoot();
}

#endif // PS2_PLATFORM
