#pragma once

#include <string>

namespace PlatformStorage
{
    std::string normalizeSlashes(const std::string& path);
    std::string normalizeRelativePath(const std::string& path);
    std::string join(const std::string& root, const std::string& relativePath);
    std::string parent(const std::string& path);
    bool hasPrefix(const std::string& path, const char* prefix);
}
