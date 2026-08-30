#ifdef PS2_PLATFORM

#include "platform/Log.h"
#include "ps2/storage/assets/Ps2AssetLocator.h"
#include "ps2/storage/Ps2Storage.h"
#include "platform/storage/PathUtils.h"
#include "platform/storage/PosixFileSystem.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <string>
#include <vector>

#include <delaythread.h>
#include <unistd.h>

namespace
{

const char PROBE_KEY[] = "assets/terrain.png";
const int USB_RETRY_MS = 3000;
const int USB_RETRY_STEP_MS = 100;

struct Candidate
{
    std::string dataRoot;
    Ps2AssetLocator::Source source;
};

struct LocatorState
{
    bool initialized = false;
    bool resolved = false;
    bool readyProbeAttempted = false;
    bool overrideSpecified = false;
    std::vector<Candidate> preferred;
    std::vector<Candidate> fallbacks;
    Ps2AssetLocator::Result result;
};

LocatorState& state()
{
    static LocatorState value;
    return value;
}

bool hasDevicePrefix(const std::string& path)
{
    const std::size_t colon = path.find(':');
    return colon != std::string::npos && colon > 0;
}

std::string uppercaseFilesystemPath(const std::string& path)
{
    std::string result = path;
    const std::size_t colon = result.find(':');
    const std::size_t begin = colon == std::string::npos ? 0 : colon + 1;
    for (std::size_t i = begin; i < result.size(); ++i)
        result[i] = static_cast<char>(std::toupper(static_cast<unsigned char>(result[i])));
    return result;
}

bool sameCandidate(const Candidate& left, const Candidate& right)
{
    return left.dataRoot == right.dataRoot;
}

void addCandidate(std::vector<Candidate>& candidates,
                  const std::string& dataRoot,
                  Ps2AssetLocator::Source source)
{
    if (dataRoot.empty())
        return;

    Candidate candidate{ PlatformStorage::normalizeSlashes(dataRoot), source };
    if (std::find_if(candidates.begin(), candidates.end(),
                     [&](const Candidate& existing) { return sameCandidate(existing, candidate); }) == candidates.end())
        candidates.push_back(candidate);
}

void addInstallCandidate(std::vector<Candidate>& candidates,
                         const std::string& installRoot,
                         Ps2AssetLocator::Source source)
{
    if (installRoot.empty())
        return;

    std::string root = PlatformStorage::normalizeSlashes(installRoot);
    if (!root.empty() && root.back() == ':' && source != Ps2AssetLocator::Source::Host)
        root.push_back('/');

    addCandidate(candidates, PlatformStorage::join(root, "data"), source);
    if (source == Ps2AssetLocator::Source::Disc)
        addCandidate(candidates, PlatformStorage::join(root, "DATA"), source);
}

Ps2AssetLocator::Source classifyPath(const std::string& path)
{
    if (PlatformStorage::hasPrefix(path, "mass"))
        return Ps2AssetLocator::Source::UsbMass;
    if (PlatformStorage::hasPrefix(path, "pfs") || PlatformStorage::hasPrefix(path, "hdd"))
        return Ps2AssetLocator::Source::HardDisk;
    if (PlatformStorage::hasPrefix(path, "cdrom") || PlatformStorage::hasPrefix(path, "cdfs:"))
        return Ps2AssetLocator::Source::Disc;
    if (PlatformStorage::hasPrefix(path, "host:"))
        return Ps2AssetLocator::Source::Host;
    return Ps2AssetLocator::Source::LaunchDevice;
}

std::string dataRootOverride(int argc, char* argv[])
{
    for (int i = 1; i < argc; ++i)
    {
        if (!argv || !argv[i])
            continue;

        const std::string argument(argv[i]);
        const std::string prefix = "--data-root=";
        if (argument.rfind(prefix, 0) == 0)
            return argument.substr(prefix.size());

        if (argument == "--data-root" && i + 1 < argc && argv[i + 1])
            return argv[i + 1];
    }
    return std::string();
}

void addLaunchCandidates(int argc, char* argv[])
{
    const std::string overrideRoot = dataRootOverride(argc, argv);
    if (!overrideRoot.empty())
    {
        state().overrideSpecified = true;
        addCandidate(state().preferred, overrideRoot, Ps2AssetLocator::Source::Override);
        MC_LOG_INFO("assets", "[PS2][assets] data override: %s\n", overrideRoot.c_str());
        return;
    }

    if (argc > 0 && argv && argv[0] && argv[0][0] != '\0')
    {
        const std::string executablePath = PlatformStorage::normalizeSlashes(argv[0]);
        if (hasDevicePrefix(executablePath))
        {
            const std::string launchDirectory = PlatformStorage::parent(executablePath);
            addInstallCandidate(state().preferred, launchDirectory, classifyPath(executablePath));
            MC_LOG_INFO("assets", "[PS2][assets] launch directory: %s\n", launchDirectory.c_str());
        }
    }

    char cwd[512] = {};
    if (::getcwd(cwd, sizeof(cwd)) && hasDevicePrefix(cwd))
    {
        const std::string currentDirectory = PlatformStorage::normalizeSlashes(cwd);
        addInstallCandidate(state().preferred, currentDirectory, classifyPath(currentDirectory));
    }
}

void addMountedFallbacks()
{
    for (int unit = 0; unit < 10; ++unit)
    {
        const std::string pfsRoot = "pfs" + std::to_string(unit) + ":/";
        addInstallCandidate(state().fallbacks, pfsRoot, Ps2AssetLocator::Source::HardDisk);
        addInstallCandidate(state().fallbacks, PlatformStorage::join(pfsRoot, "MCBETA"), Ps2AssetLocator::Source::HardDisk);
    }

    addInstallCandidate(state().fallbacks, "cdfs:/", Ps2AssetLocator::Source::Disc);
    addInstallCandidate(state().fallbacks, "cdfs:/MCBETA", Ps2AssetLocator::Source::Disc);
    addInstallCandidate(state().fallbacks, "cdrom0:/", Ps2AssetLocator::Source::Disc);
    addInstallCandidate(state().fallbacks, "cdrom0:/MCBETA", Ps2AssetLocator::Source::Disc);
    addInstallCandidate(state().fallbacks, "host:", Ps2AssetLocator::Source::Host);
    addInstallCandidate(state().fallbacks, "host:MCBETA", Ps2AssetLocator::Source::Host);
}

bool isDiscPath(const std::string& path, Ps2AssetLocator::Source source)
{
    return source == Ps2AssetLocator::Source::Disc ||
           PlatformStorage::hasPrefix(path, "cdrom") ||
           PlatformStorage::hasPrefix(path, "cdfs:");
}

std::string resolveDiscFile(const std::string& path)
{
    if (PlatformStorage::fileReadable(path))
        return path;

    const std::string upper = uppercaseFilesystemPath(path);
    if (PlatformStorage::fileReadable(upper))
        return upper;

    const std::string versioned = upper + ";1";
    if (PlatformStorage::fileReadable(versioned))
        return versioned;

    std::string nativePath = upper;
    const std::size_t colon = nativePath.find(':');
    const std::size_t begin = colon == std::string::npos ? 0 : colon + 1;
    for (std::size_t i = begin; i < nativePath.size(); ++i)
        if (nativePath[i] == '/')
            nativePath[i] = '\\';

    if (PlatformStorage::fileReadable(nativePath))
        return nativePath;
    nativePath += ";1";
    return PlatformStorage::fileReadable(nativePath) ? nativePath : std::string();
}

std::string resolveCandidateFile(const Candidate& candidate, const std::string& key)
{
    const std::string path = PlatformStorage::join(candidate.dataRoot, key);
    if (isDiscPath(candidate.dataRoot, candidate.source))
        return resolveDiscFile(path);
    return PlatformStorage::fileReadable(path) ? path : std::string();
}

bool selectFrom(const std::vector<Candidate>& candidates)
{
    for (const Candidate& candidate : candidates)
    {
        if (resolveCandidateFile(candidate, PROBE_KEY).empty())
            continue;

        state().result.dataRoot = candidate.dataRoot;
        state().result.source = candidate.source;
        state().resolved = true;
        MC_LOG_INFO("assets", "[PS2][assets] data directory: %s (%s)\n",
                    state().result.dataRoot.c_str(),
                    Ps2AssetLocator::sourceName(state().result.source));
        return true;
    }
    return false;
}

bool probeUsb()
{
    const std::string massRoot = Ps2Storage::massRoot();
    if (massRoot.empty())
        return false;

    std::vector<Candidate> candidates;
    addInstallCandidate(candidates, massRoot, Ps2AssetLocator::Source::UsbMass);
    addInstallCandidate(candidates, PlatformStorage::join(massRoot, "MCBETA"), Ps2AssetLocator::Source::UsbMass);
    return selectFrom(candidates);
}

bool probeOnce()
{
    return selectFrom(state().preferred) || probeUsb() || selectFrom(state().fallbacks);
}

void printMissingData()
{
    MC_LOG_INFO("assets", "[PS2][assets] NO DATA FOUND. Use --data-root=<path> or place data/ beside the ELF.\n");
    for (const Candidate& candidate : state().preferred)
        MC_LOG_INFO("assets", "[PS2][assets]   %s\n", PlatformStorage::join(candidate.dataRoot, PROBE_KEY).c_str());
}

void ensureInitialized()
{
    if (state().initialized)
        return;
    state().initialized = true;
    addMountedFallbacks();
}

} // namespace

namespace Ps2AssetLocator
{

void init(int argc, char* argv[])
{
    ensureInitialized();
    addLaunchCandidates(argc, argv);

    if (!state().resolved || state().preferred.empty())
        return;

    const bool currentIsPreferred = std::find_if(
        state().preferred.begin(), state().preferred.end(),
        [](const Candidate& candidate) { return candidate.dataRoot == state().result.dataRoot; }) != state().preferred.end();

    if (!currentIsPreferred || (state().overrideSpecified && state().result.source != Source::Override))
    {
        state().resolved = false;
        state().result = Result{};
        state().readyProbeAttempted = false;
    }
}

bool resolve(Result& out)
{
    ensureInitialized();

    if (state().resolved)
    {
        out = state().result;
        return true;
    }

    if (state().overrideSpecified)
    {
        if (selectFrom(state().preferred))
        {
            out = state().result;
            return true;
        }
        if (Ps2Storage::fileIoReady() && !state().readyProbeAttempted)
        {
            state().readyProbeAttempted = true;
            MC_LOG_INFO("assets", "[PS2][assets] explicit data root is not readable\n");
            printMissingData();
        }
        return false;
    }

    if (probeOnce())
    {
        out = state().result;
        return true;
    }

    if (!Ps2Storage::fileIoReady())
        return false;

    if (!state().readyProbeAttempted)
    {
        state().readyProbeAttempted = true;
        for (int waited = 0; waited < USB_RETRY_MS; waited += USB_RETRY_STEP_MS)
        {
            DelayThread(USB_RETRY_STEP_MS * 1000);
            if (probeUsb())
            {
                out = state().result;
                return true;
            }
        }

        if (probeOnce())
        {
            out = state().result;
            return true;
        }
        printMissingData();
    }

    return false;
}

std::string resolveFile(const std::string& dataRoot, Source source, const std::string& key)
{
    Candidate candidate{ dataRoot, source };
    return resolveCandidateFile(candidate, key);
}

std::string resolveDirectory(const std::string& dataRoot, Source source, const std::string& key)
{
    const std::string path = PlatformStorage::join(dataRoot, key);
    if (PlatformStorage::directoryAvailable(path))
        return path;

    if (isDiscPath(dataRoot, source))
    {
        const std::string upper = uppercaseFilesystemPath(path);
        if (PlatformStorage::directoryAvailable(upper))
            return upper;
    }
    return path;
}

const char* sourceName(Source source)
{
    switch (source)
    {
        case Source::Override: return "data-root override";
        case Source::LaunchDevice: return "launch device";
        case Source::UsbMass: return "USB mass storage";
        case Source::HardDisk: return "mounted HDD/PFS";
        case Source::Disc: return "CD/DVD";
        case Source::Host: return "host";
        default: return "unknown";
    }
}

} // namespace Ps2AssetLocator

#endif // PS2_PLATFORM
