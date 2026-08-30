#include "platform/storage/PathUtils.h"

namespace PlatformStorage
{

std::string normalizeSlashes(const std::string& path)
{
    std::string result = path;
    for (char& c : result)
        if (c == '\\')
            c = '/';
    return result;
}

std::string normalizeRelativePath(const std::string& path)
{
    std::string result = normalizeSlashes(path);
    while (!result.empty() && result.front() == '/')
        result.erase(result.begin());
    return result;
}

std::string join(const std::string& root, const std::string& relativePath)
{
    const std::string relative = normalizeRelativePath(relativePath);
    if (relative.empty())
        return normalizeSlashes(root);
    if (root.empty())
        return relative;

    std::string result = normalizeSlashes(root);
    if (result.back() != '/' && result.back() != ':')
        result.push_back('/');
    return result + relative;
}

std::string parent(const std::string& path)
{
    const std::string normalized = normalizeSlashes(path);
    const std::size_t pos = normalized.find_last_of('/');
    if (pos == std::string::npos)
        return std::string();
    return normalized.substr(0, pos);
}

bool hasPrefix(const std::string& path, const char* prefix)
{
    if (!prefix)
        return false;
    const std::string normalized = normalizeSlashes(path);
    const std::string prefixString(prefix);
    return normalized.rfind(prefixString, 0) == 0;
}

} // namespace PlatformStorage
