#pragma once

#ifdef PS2_PLATFORM

#include <cstddef>
#include <string>
#include <vector>

namespace Ps2MemoryCardFileSystem
{
    bool handles(const std::string& path);
    bool mkdirs(const std::string& path);
    bool writeFile(const std::string& path, const void* data, std::size_t length);
    bool readFile(const std::string& path, std::vector<unsigned char>& out);
    bool exists(const std::string& path);
    bool removeFile(const std::string& path);
    bool listEntries(const std::string& path, std::vector<std::string>& out);
}

#endif // PS2_PLATFORM
