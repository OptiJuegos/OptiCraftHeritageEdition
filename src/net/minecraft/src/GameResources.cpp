#include "GameResources.h"
#include "platform/Resources.h"

namespace GameResources
{
std::string getExeDir() { return PlatformResources::baseDir(); }
std::string getAssetsDir() { return PlatformResources::assetsDir(); }
std::string getAudioResourcesDir() { return PlatformResources::audioDir(); }
std::string resolve(const std::string& mcPath) { return PlatformResources::resolveAsset(mcPath); }

std::unique_ptr<std::ifstream> open(const std::string& mcPath)
{
    std::string resolved = resolve(mcPath);
#ifdef PS2_PLATFORM
    if (resolved.empty())
    {
        std::string fallback = mcPath;
        if (!fallback.empty() && fallback[0] == '/') fallback.erase(fallback.begin());
        resolved = PlatformResources::resolveExisting(fallback);
    }
#endif
    if (resolved.empty()) return nullptr;
    auto input = std::make_unique<std::ifstream>(resolved, std::ios::binary);
    return input->good() ? std::move(input) : nullptr;
}
}
