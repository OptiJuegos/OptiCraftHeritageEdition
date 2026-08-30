#include "platform/Resources.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <SDL2/SDL.h>

std::string PlatformResources::baseDir()
{
    char* base = SDL_GetBasePath();
    if (base)
    {
        std::string out(base);
        SDL_free(base);
        return std::filesystem::path(out).lexically_normal().string();
    }
    return std::filesystem::current_path().string();
}

std::string PlatformResources::assetsDir()
{
    return (std::filesystem::path(baseDir()) / "assets").string();
}

std::string PlatformResources::audioDir()
{
    return (std::filesystem::path(baseDir()) / "resources").string();
}

std::string PlatformResources::resolveExisting(const std::string& path)
{
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
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    return file ? static_cast<long>(file.tellg()) : -1L;
}

unsigned char* PlatformResources::loadFile(const std::string& path, unsigned int* outSize)
{
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
