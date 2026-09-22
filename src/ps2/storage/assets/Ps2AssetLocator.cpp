#ifdef PS2_PLATFORM

#include "platform/Log.h"
#include "ps2/storage/assets/Ps2AssetLocator.h"
#include "ps2/storage/Ps2Storage.h"
#include "ps2/tuning/Ps2CoreTuning.h"
#include "platform/storage/AssetPak.h"
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
const char PAK_NAME[] = "assets.pak";
const int USB_RETRY_MS = 3000;
const int USB_RETRY_STEP_MS = 100;

using Ps2AssetLocator::INSTALL_FOLDER;

// Upper bound on names probeByScan() will look at in one device root. A device
// root holding more directories than this is not an install layout, and
// enumeration is the operation worth spending the least on here.
const int SCAN_MAX_ENTRIES = 64;

struct Candidate
{
    std::string dataRoot;
    Ps2AssetLocator::Source source;
};

struct LocatorState
{
    bool initialized = false;
    bool resolved = false;
    bool readyResolutionComplete = false;
    bool overrideSpecified = false;
    // Set once addLaunchCandidates() sees the ELF was itself loaded from
    // cdrom0/cdfs. A disc build's data/ always ships on the same disc as the
    // ELF -- there is never a reason for it to also live on a USB/mass device
    // -- so this skips probeUsb() and the USB settle retry entirely for a disc
    // launch, instead of relying on the preferred-candidate probe merely
    // succeeding first.
    bool launchedFromDisc = false;
    bool scanAttempted = false;
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

std::string canonicalCandidateKey(const Candidate& candidate)
{
    std::string key = PlatformStorage::normalizeSlashes(candidate.dataRoot);
    while (key.size() > 1 && key.back() == '/')
        key.pop_back();

    if (key.rfind("host:/", 0) == 0 && key.size() > 7 &&
        std::isalpha(static_cast<unsigned char>(key[6])) && key[7] == ':')
        key.erase(5, 1);

    return key;
}

bool sameCandidate(const Candidate& left, const Candidate& right)
{
    return canonicalCandidateKey(left) == canonicalCandidateKey(right);
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
    if (source == Ps2AssetLocator::Source::Host)
    {
        if (!root.empty() && root.back() == ':')
            addCandidate(candidates, root + "/data", source);
    }
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
            std::string launchDirectory = PlatformStorage::parent(executablePath);
            if (launchDirectory.empty())
            {
                const std::size_t colon = executablePath.find(':');
                if (colon != std::string::npos)
                    launchDirectory = executablePath.substr(0, colon + 1);
            }
            const Ps2AssetLocator::Source launchSource = classifyPath(executablePath);
            addInstallCandidate(state().preferred, launchDirectory, launchSource);
            if (launchSource == Ps2AssetLocator::Source::Disc)
                state().launchedFromDisc = true;
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
    addInstallCandidate(state().fallbacks, "cdrom0:/", Ps2AssetLocator::Source::Disc);
    addInstallCandidate(state().fallbacks, std::string("cdrom0:/") + INSTALL_FOLDER, Ps2AssetLocator::Source::Disc);
    addInstallCandidate(state().fallbacks, "cdfs:/", Ps2AssetLocator::Source::Disc);
    addInstallCandidate(state().fallbacks, std::string("cdfs:/") + INSTALL_FOLDER, Ps2AssetLocator::Source::Disc);

#if PS2_SUPPORT_HDD
    for (int unit = 0; unit < 10; ++unit)
    {
        const std::string pfsRoot = "pfs" + std::to_string(unit) + ":/";
        addInstallCandidate(state().fallbacks, pfsRoot, Ps2AssetLocator::Source::HardDisk);
        addInstallCandidate(state().fallbacks, PlatformStorage::join(pfsRoot, INSTALL_FOLDER), Ps2AssetLocator::Source::HardDisk);
    }
#endif
    addInstallCandidate(state().fallbacks, "host:", Ps2AssetLocator::Source::Host);
    addInstallCandidate(state().fallbacks, "host:/", Ps2AssetLocator::Source::Host);
    addInstallCandidate(state().fallbacks, std::string("host:") + INSTALL_FOLDER, Ps2AssetLocator::Source::Host);
    addInstallCandidate(state().fallbacks, std::string("host:/") + INSTALL_FOLDER, Ps2AssetLocator::Source::Host);
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

// An install packed with scripts/make_pak.py may ship assets.pak beside
// data/ and no loose data/assets at all. The pak sits in the install root,
// i.e. the parent of the candidate's data root. The probe IS the mount: a
// root only counts if AssetPak can open and parse the pak there, which is
// what Resources_PS2 goes on to read from -- a device that answers fopen()
// but not the read (PCSX2's host: with an absolute path) cannot pass.
static std::vector<std::string> s_diagnosticLogs;

bool candidateHasPak(const Candidate& candidate)
{
    if (!Ps2Storage::fileIoReady())
    {
        Ps2AssetLocator::addDiagnostic("candidateHasPak: fileIoReady is false");
        return false;
    }
    std::string installRoot = PlatformStorage::parent(candidate.dataRoot);
    if (installRoot.empty())
    {
        const std::size_t colon = candidate.dataRoot.find(':');
        if (colon == std::string::npos)
            return false;
        installRoot = candidate.dataRoot.substr(0, colon + 1);
    }
    if (isDiscPath(candidate.dataRoot, candidate.source))
    {
        const std::string pakPath = resolveDiscFile(PlatformStorage::join(installRoot, PAK_NAME));
        if (pakPath.empty())
            return false;
        bool ok = AssetPak::mountFile(pakPath);
        Ps2AssetLocator::addDiagnostic("mountDisc(" + pakPath + "): " + (ok ? "OK" : "FAIL"));
        return ok;
    }
    if (AssetPak::mountFrom(installRoot))
    {
        Ps2AssetLocator::addDiagnostic("mountFrom(" + installRoot + "): OK");
        return true;
    }
    if (!installRoot.empty() && installRoot.back() == ':')
    {
        if (AssetPak::mountFrom(installRoot + "/"))
        {
            Ps2AssetLocator::addDiagnostic("mountFrom(" + installRoot + "/): OK");
            return true;
        }
    }
    if (candidate.source == Ps2AssetLocator::Source::Host || candidate.dataRoot.find("host:") != std::string::npos)
    {
        if (AssetPak::mountFile("host:assets.pak"))
        {
            Ps2AssetLocator::addDiagnostic("mountFile(host:assets.pak): OK");
            return true;
        }
        if (AssetPak::mountFile("host:/assets.pak"))
        {
            Ps2AssetLocator::addDiagnostic("mountFile(host:/assets.pak): OK");
            return true;
        }
        if (AssetPak::mountFile("assets.pak"))
        {
            Ps2AssetLocator::addDiagnostic("mountFile(assets.pak): OK");
            return true;
        }
    }
    Ps2AssetLocator::addDiagnostic("mountFrom(" + installRoot + "): FAIL");
    return false;
}

bool selectFrom(const std::vector<Candidate>& candidates)
{
    for (const Candidate& candidate : candidates)
    {
        if (resolveCandidateFile(candidate, PROBE_KEY).empty() && !candidateHasPak(candidate))
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

bool probeUsbRoot(const std::string& massRoot)
{
    if (massRoot.empty())
        return false;

    std::vector<Candidate> candidates;
    addInstallCandidate(candidates, massRoot, Ps2AssetLocator::Source::UsbMass);
    addInstallCandidate(candidates, PlatformStorage::join(massRoot, INSTALL_FOLDER), Ps2AssetLocator::Source::UsbMass);
    return selectFrom(candidates);
}

bool probeUsbWithRetry(std::string& massRoot)
{
    Ps2Storage::MassStorageProbe probe = Ps2Storage::probeMassStorage(false);
    if (!probe.driverAvailable)
        return false;

    if (!probe.root.empty())
    {
        massRoot = probe.root;
        return probeUsbRoot(massRoot);
    }

    for (int waited = 0; waited < USB_RETRY_MS; waited += USB_RETRY_STEP_MS)
    {
        DelayThread(USB_RETRY_STEP_MS * 1000);
        probe = Ps2Storage::probeMassStorage(false);
        if (!probe.driverAvailable)
            return false;
        if (!probe.root.empty())
        {
            massRoot = probe.root;
            return probeUsbRoot(massRoot);
        }
    }

    probe = Ps2Storage::probeMassStorage(true);
    massRoot = probe.root;
    return !massRoot.empty() && probeUsbRoot(massRoot);
}

// Last resort, and deliberately last: everything above finds the install by
// asking for one exact path, which is the only thing that works everywhere.
// This is the only probe that enumerates a directory, and enumeration is the
// unreliable operation on this console -- it never returns on a disc (see
// Ps2ResourceManifest) and usbhdfsd has been observed hanging on a folder with
// enough entries. So it runs only once every named guess has already missed,
// never on a disc path, and reads at most SCAN_MAX_ENTRIES names before giving
// up rather than walking an arbitrarily large device root.
//
// What it buys: the install folder no longer has to be called INSTALL_FOLDER.
// Any single directory holding data/assets/terrain.png is accepted, so a user
// who renamed it -- or a launcher that hands over an argv[0] this code cannot
// use -- still boots.
bool probeByScan(const std::string& deviceRoot, Ps2AssetLocator::Source source)
{
    if (deviceRoot.empty() || isDiscPath(deviceRoot, source))
        return false;

    std::vector<std::string> entries;
    if (!PlatformStorage::listEntries(deviceRoot, entries))
        return false;

    int examined = 0;
    for (const std::string& entry : entries)
    {
        if (entry.empty() || entry == "." || entry == "..")
            continue;
        if (++examined > SCAN_MAX_ENTRIES)
        {
            MC_LOG_INFO("assets", "[PS2][assets] scan of %s stopped at %d entries\n",
                        deviceRoot.c_str(), SCAN_MAX_ENTRIES);
            break;
        }

        std::vector<Candidate> candidates;
        addInstallCandidate(candidates, PlatformStorage::join(deviceRoot, entry), source);
        if (selectFrom(candidates))
        {
            MC_LOG_INFO("assets", "[PS2][assets] found install by scan: %s\n", entry.c_str());
            return true;
        }
    }
    return false;
}

bool probeScanRoots(const std::string& massRoot)
{
    if (state().scanAttempted)
        return false;
    state().scanAttempted = true;

    if (probeByScan(massRoot, Ps2AssetLocator::Source::UsbMass))
        return true;

#if PS2_SUPPORT_HDD
    for (int unit = 0; unit < 10; ++unit)
    {
        const std::string pfsRoot = "pfs" + std::to_string(unit) + ":/";
        if (PlatformStorage::directoryAvailable(pfsRoot) &&
            probeByScan(pfsRoot, Ps2AssetLocator::Source::HardDisk))
            return true;
    }
#endif
    return false;
}

bool probeStaticCandidates()
{
    std::vector<Candidate> candidates = state().preferred;
    for (const Candidate& fallback : state().fallbacks)
        addCandidate(candidates, fallback.dataRoot, fallback.source);
    return selectFrom(candidates);
}

bool probeBeforeFileIo()
{
    if (probeStaticCandidates())
        return true;

    const Ps2Storage::MassStorageProbe mass = Ps2Storage::probeMassStorage(true);
    return !mass.root.empty() && probeUsbRoot(mass.root);
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

    const Candidate current{ state().result.dataRoot, state().result.source };
    const bool currentIsPreferred = std::find_if(
        state().preferred.begin(), state().preferred.end(),
        [&](const Candidate& candidate) { return sameCandidate(candidate, current); }) != state().preferred.end();

    if (!currentIsPreferred || (state().overrideSpecified && state().result.source != Source::Override))
    {
        state().resolved = false;
        state().result = Result{};
        state().readyResolutionComplete = false;
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

    if (!Ps2Storage::fileIoReady())
    {
        const bool found = state().overrideSpecified
            ? selectFrom(state().preferred)
            : probeBeforeFileIo();
        if (!found)
            return false;

        out = state().result;
        return true;
    }

    if (state().readyResolutionComplete)
        return false;
    state().readyResolutionComplete = true;

    if (state().overrideSpecified)
    {
        if (selectFrom(state().preferred))
        {
            out = state().result;
            return true;
        }
        MC_LOG_INFO("assets", "[PS2][assets] explicit data root is not readable\n");
        printMissingData();
        return false;
    }

    if (probeStaticCandidates())
    {
        out = state().result;
        return true;
    }

    if (!state().launchedFromDisc)
    {
        std::string massRoot;
        if (probeUsbWithRetry(massRoot))
        {
            out = state().result;
            return true;
        }
        if (probeScanRoots(massRoot))
        {
            out = state().result;
            return true;
        }
    }

    printMissingData();
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

    // opendir() on the disc's IOP CD driver is not the reliable exact-case
    // gate fopen() is: given the exact-case (lowercase) path it returns a
    // valid, non-null handle regardless -- but that handle then enumerates as
    // empty, because the directory record it actually needs to walk only
    // exists under the ISO9660 UPPERCASE name. directoryAvailable() alone
    // cannot tell a real match from that false positive, so for a disc path
    // check the case the ISO authoring tool actually writes (upper) first.
    // Resolving "resources" to the lowercase path here is exactly what
    // silently made ThreadDownloadResources scan zero files, leaving audsrv
    // with nothing to play.
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

const std::vector<std::string>& diagnosticLogs()
{
    return s_diagnosticLogs;
}

void addDiagnostic(const std::string& msg)
{
    if (s_diagnosticLogs.size() < 20)
        s_diagnosticLogs.push_back(msg);
}

} // namespace Ps2AssetLocator

#endif // PS2_PLATFORM
