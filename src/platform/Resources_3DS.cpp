#include "platform/Resources.h"
#include "platform/storage/AssetPak.h"
#include "3ds/DsBootstrap.h"

#include <cstdlib>
#include <fstream>

std::string PlatformResources::baseDir()
{
    return dsGetAppDir();
}

std::string PlatformResources::assetsDir()
{
    return baseDir() + "/data/assets";
}

std::string PlatformResources::audioDir()
{
    return baseDir() + "/data/resources";
}

std::string PlatformResources::resolveExisting(const std::string& path)
{
    // <app>/assets.pak, keyed the way `path` is spelled here ("assets/...",
    // "resources/..."); a hit answers with a pak:// path and skips the fsdev
    // ("sdmc:/...") directory walk a loose open would do.
    if (AssetPak::mountFrom(baseDir()) && AssetPak::exists(path))
        return AssetPak::makePath(path);

    std::string resolved;
    if (path.rfind("assets/", 0) == 0)
        resolved = assetsDir() + "/" + path.substr(7);
    else
        resolved = baseDir() + "/" + path;

    std::ifstream file(resolved, std::ios::binary);
    return file.good() ? resolved : std::string();
}

std::string PlatformResources::resolveAsset(const std::string& input)
{
    std::string path = input;
    if (!path.empty() && path[0] == '/')
        path.erase(path.begin());
    return resolveExisting("assets/" + path);
}

long PlatformResources::fileSize(const std::string& path)
{
    if (AssetPak::isPakPath(path))
        return AssetPak::size(AssetPak::keyOf(path));
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    return file ? static_cast<long>(file.tellg()) : -1L;
}

unsigned char* PlatformResources::loadFile(const std::string& path, unsigned int* outSize)
{
    if (AssetPak::isPakPath(path))
        return AssetPak::load(AssetPak::keyOf(path), outSize);
    if (outSize)
        *outSize = 0;
    const long size = fileSize(path);
    if (size <= 0)
        return nullptr;

    unsigned char* data = static_cast<unsigned char*>(std::malloc(static_cast<std::size_t>(size)));
    if (!data)
        return nullptr;

    std::ifstream file(path, std::ios::binary);
    if (!file.read(reinterpret_cast<char*>(data), size))
    {
        std::free(data);
        return nullptr;
    }
    if (outSize)
        *outSize = static_cast<unsigned int>(size);
    return data;
}
