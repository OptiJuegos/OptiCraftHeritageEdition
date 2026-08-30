#pragma once

#ifdef PS2_PLATFORM

#include <string>

namespace Ps2AssetLocator
{
    enum class Source
    {
        Unknown,
        Override,
        LaunchDevice,
        UsbMass,
        HardDisk,
        Disc,
        Host
    };

    struct Result
    {
        std::string dataRoot;
        Source source = Source::Unknown;
    };

    void init(int argc, char* argv[]);
    bool resolve(Result& out);
    std::string resolveFile(const std::string& dataRoot, Source source, const std::string& key);
    std::string resolveDirectory(const std::string& dataRoot, Source source, const std::string& key);
    const char* sourceName(Source source);
}

#endif // PS2_PLATFORM
