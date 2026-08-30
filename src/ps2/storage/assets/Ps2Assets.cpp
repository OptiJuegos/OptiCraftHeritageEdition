#ifdef PS2_PLATFORM

#include "platform/Log.h"
#include "ps2/storage/assets/Ps2Assets.h"
#include "ps2/storage/Ps2Storage.h"
#include "platform/storage/PathUtils.h"
#include "platform/storage/PosixFileSystem.h"

#include <cstdio>
#include <string>

namespace
{

struct AssetState
{
    bool available = false;
    std::string dataDir;
    Ps2Assets::Source source = Ps2Assets::Source::Unknown;
};

AssetState& state()
{
    static AssetState value;
    return value;
}

void ensureResolved()
{
    if (state().available)
        return;

    Ps2AssetLocator::Result result;
    if (!Ps2AssetLocator::resolve(result))
        return;

    state().available = true;
    state().dataDir = result.dataRoot;
    state().source = result.source;
}

unsigned char* loadResolvedPath(const std::string& path, unsigned int* outSize)
{
    unsigned char* data = Ps2Storage::readWholeFile(path, outSize, 64);
    if (!data)
        MC_LOG_ERROR("assets", "[PS2][assets] failed to read %s\n", path.c_str());
    return data;
}

} // namespace

namespace Ps2Assets
{

void init(int argc, char* argv[])
{
    Ps2AssetLocator::init(argc, argv);
    state().available = false;
    state().dataDir.clear();
    state().source = Source::Unknown;
    ensureResolved();
}

bool available()
{
    ensureResolved();
    return state().available;
}

const char* dataDir()
{
    ensureResolved();
    return state().dataDir.c_str();
}

Source source()
{
    ensureResolved();
    return state().source;
}

const char* sourceName()
{
    ensureResolved();
    return Ps2AssetLocator::sourceName(state().source);
}

std::string assetDir()
{
    ensureResolved();
    return Ps2AssetLocator::resolveDirectory(state().dataDir, state().source, "assets");
}

std::string resourcesDir()
{
    ensureResolved();
    return Ps2AssetLocator::resolveDirectory(state().dataDir, state().source, "resources");
}

std::string resolve(const std::string& key)
{
    ensureResolved();
    return PlatformStorage::join(state().dataDir, key);
}

std::string resolveExisting(const std::string& key)
{
    ensureResolved();
    return Ps2AssetLocator::resolveFile(state().dataDir, state().source, key);
}

long fileSize(const std::string& path)
{
    ensureResolved();
    return static_cast<long>(PlatformStorage::fileSize(path));
}

unsigned char* loadAsset(const std::string& key, unsigned int* outSize)
{
    const std::string path = resolveExisting(key);
    if (path.empty())
    {
        if (outSize)
            *outSize = 0;
        MC_LOG_INFO("assets", "[PS2][assets] missing %s\n", key.c_str());
        return nullptr;
    }
    return loadResolvedPath(path, outSize);
}

unsigned char* loadFile(const std::string& path, unsigned int* outSize)
{
    ensureResolved();
    return loadResolvedPath(path, outSize);
}

} // namespace Ps2Assets

#endif // PS2_PLATFORM
