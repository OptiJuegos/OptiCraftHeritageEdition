#include "platform/Resources.h"
#include "ps2/storage/assets/Ps2Assets.h"

std::string PlatformResources::baseDir()
{
    return Ps2Assets::dataDir();
}

std::string PlatformResources::assetsDir()
{
    return Ps2Assets::assetDir();
}

std::string PlatformResources::audioDir()
{
    return Ps2Assets::resourcesDir();
}

std::string PlatformResources::resolveExisting(const std::string& path)
{
    return Ps2Assets::resolveExisting(path);
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
    return Ps2Assets::fileSize(path);
}

unsigned char* PlatformResources::loadFile(const std::string& path, unsigned int* outSize)
{
    return Ps2Assets::loadFile(path, outSize);
}
