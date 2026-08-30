#include "platform/Storage.h"
#include "platform/storage/PathUtils.h"

#include <filesystem>
#include <fstream>

namespace PlatformStorage
{
bool exists(const std::string& path)
{
    std::error_code ec;
    return std::filesystem::exists(path, ec);
}

bool mkdirs(const std::string& path)
{
    std::error_code ec;
    if (path.empty())
        return false;
    if (std::filesystem::exists(path, ec))
        return true;
    return std::filesystem::create_directories(path, ec) || std::filesystem::exists(path, ec);
}

bool removeFile(const std::string& path)
{
    std::error_code ec;
    return std::filesystem::remove(path, ec);
}

bool renameFile(const std::string& from, const std::string& to)
{
    std::error_code ec;
    std::filesystem::rename(from, to, ec);
    return !ec;
}

bool supportsAtomicRename() { return true; }
bool supportsSessionLocks() { return true; }

bool readFile(const std::string& path, std::vector<unsigned char>& out)
{
    std::ifstream file(path, std::ios::binary);
    if (!file)
        return false;
    file.seekg(0, std::ios::end);
    const std::streamoff size = file.tellg();
    if (size < 0)
        return false;
    file.seekg(0, std::ios::beg);
    out.resize(static_cast<std::size_t>(size));
    if (size > 0)
        file.read(reinterpret_cast<char*>(out.data()), size);
    return file.good() || file.eof();
}

bool writeFile(const std::string& path, const void* data, std::size_t length)
{
    const std::string parentDir = parent(path);
    if (!parentDir.empty() && !mkdirs(parentDir))
        return false;
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (!file)
        return false;
    if (length != 0)
        file.write(static_cast<const char*>(data), static_cast<std::streamsize>(length));
    return file.good();
}

bool pathIsDirectory(const std::string& path)
{
    std::error_code ec;
    return std::filesystem::is_directory(path, ec);
}

bool listPathEntries(const std::string& path, std::vector<std::string>& out)
{
    out.clear();
    std::error_code ec;
    for (const auto& entry : std::filesystem::directory_iterator(path, ec))
    {
        if (ec)
            break;
        out.push_back(entry.path().filename().string());
    }
    return !ec;
}

bool listDirs(const std::string& path, std::vector<std::string>& out)
{
    out.clear();
    std::error_code ec;
    for (const auto& entry : std::filesystem::directory_iterator(path, ec))
    {
        if (ec)
            break;
        if (entry.is_directory())
            out.push_back(entry.path().filename().string());
    }
    return !ec;
}
}
