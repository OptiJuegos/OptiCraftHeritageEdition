#pragma once

#include <string>

namespace PlatformResources
{
std::string baseDir();
std::string assetsDir();
std::string audioDir();
std::string resolveAsset(const std::string& path);
std::string resolveExisting(const std::string& path);
long fileSize(const std::string& path);
unsigned char* loadFile(const std::string& path, unsigned int* outSize);
}
