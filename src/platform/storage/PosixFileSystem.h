#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace PlatformStorage
{
    bool directoryAvailable(const std::string& path);
    bool fileReadable(const std::string& path);
    bool createFile(const std::string& path);
    bool removePath(const std::string& path);
    bool renamePath(const std::string& source, const std::string& destination);
    bool posixExists(const std::string& path);
    bool isDirectory(const std::string& path);
    bool isFile(const std::string& path);
    std::int64_t lastModifiedMs(const std::string& path);
    std::int64_t fileSize(const std::string& path);
    bool makeDirectory(const std::string& path, int mode = 0777);
    bool makeDirectories(const std::string& path, int mode = 0777);
    bool listEntries(const std::string& path, std::vector<std::string>& out);
    bool posixReadFile(const std::string& path, std::vector<unsigned char>& out);
    bool posixWriteFile(const std::string& path, const void* data, std::size_t length);
}
