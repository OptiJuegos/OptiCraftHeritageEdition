#pragma once

#ifdef PS2_PLATFORM

#include <string>
#include <vector>

namespace Ps2AssetLocator
{
    // The conventional install folder name. Still tried by name on every
    // device, because asking for one exact path is the only lookup that works
    // on all of them -- but no longer the only name that can work, since the
    // locator also scans for any folder holding the probe asset. Also the
    // folder saves fall back to when the install itself is read-only.
    inline constexpr char INSTALL_FOLDER[] = "OptiCraftHeritage";

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

    const std::vector<std::string>& diagnosticLogs();
    void addDiagnostic(const std::string& msg);
}

#endif // PS2_PLATFORM
