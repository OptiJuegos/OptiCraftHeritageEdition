#if defined(PS2_PLATFORM) || defined(WII_PLATFORM)

#include "platform/storage/PosixFileSystem.h"
#include "platform/storage/PathUtils.h"

#include <cstdio>
#include <cstring>
#include <fstream>

#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

namespace PlatformStorage
{

bool directoryAvailable(const std::string& path)
{
    DIR* dir = ::opendir(path.c_str());
    if (!dir && !path.empty() && path.back() != '/')
        dir = ::opendir((path + "/").c_str());
    if (!dir)
        return false;
    ::closedir(dir);
    return true;
}

bool fileReadable(const std::string& path)
{
    FILE* file = std::fopen(path.c_str(), "rb");
    if (!file)
        return false;
    std::fclose(file);
    return true;
}

bool createFile(const std::string& path)
{
    const int fd = ::open(path.c_str(), O_CREAT | O_RDWR, 0644);
    if (fd < 0)
        return false;
    ::close(fd);
    return true;
}

bool removePath(const std::string& path)
{
    if (isDirectory(path))
        return ::rmdir(path.c_str()) == 0;
    return ::unlink(path.c_str()) == 0;
}

bool renamePath(const std::string& source, const std::string& destination)
{
    return ::rename(source.c_str(), destination.c_str()) == 0;
}

bool posixExists(const std::string& path)
{
    struct stat st{};
    return ::stat(path.c_str(), &st) == 0;
}

bool isDirectory(const std::string& path)
{
    struct stat st{};
    if (::stat(path.c_str(), &st) != 0)
        return false;
    return S_ISDIR(st.st_mode);
}

bool isFile(const std::string& path)
{
    struct stat st{};
    if (::stat(path.c_str(), &st) != 0)
        return false;
    return S_ISREG(st.st_mode);
}

std::int64_t lastModifiedMs(const std::string& path)
{
    struct stat st{};
    if (::stat(path.c_str(), &st) != 0)
        return 0;
    return static_cast<std::int64_t>(st.st_mtime) * 1000LL;
}

std::int64_t fileSize(const std::string& path)
{
    struct stat st{};
    if (::stat(path.c_str(), &st) != 0)
        return -1;
    return static_cast<std::int64_t>(st.st_size);
}

bool makeDirectory(const std::string& path, int mode)
{
    if (posixExists(path))
        return isDirectory(path);
    return ::mkdir(path.c_str(), mode) == 0;
}

bool makeDirectories(const std::string& path, int mode)
{
    const std::string normalized = normalizeSlashes(path);
    for (std::size_t i = 1; i <= normalized.size(); ++i)
    {
        if (i != normalized.size() && normalized[i] != '/')
            continue;

        const std::string directory = normalized.substr(0, i);
        if (directory.empty() || directory.back() == ':')
            continue;
        if (::mkdir(directory.c_str(), mode) != 0 && !isDirectory(directory))
            return false;
    }
    return true;
}

bool listEntries(const std::string& path, std::vector<std::string>& out)
{
    out.clear();
    DIR* dir = ::opendir(path.c_str());
    if (!dir && !path.empty() && path.back() != '/')
        dir = ::opendir((path + "/").c_str());
    if (!dir)
        return false;

    while (dirent* entry = ::readdir(dir))
    {
        if (std::strcmp(entry->d_name, ".") == 0 || std::strcmp(entry->d_name, "..") == 0)
            continue;
        out.emplace_back(entry->d_name);
    }

    ::closedir(dir);
    return true;
}

bool posixReadFile(const std::string& path, std::vector<unsigned char>& out)
{
    out.clear();
    std::ifstream input(path.c_str(), std::ios::binary);
    if (!input.good())
        return false;

    char buffer[16384];
    while (input.good())
    {
        input.read(buffer, sizeof(buffer));
        const std::streamsize count = input.gcount();
        if (count > 0)
            out.insert(out.end(), buffer, buffer + count);
    }
    return true;
}

bool posixWriteFile(const std::string& path, const void* data, std::size_t length)
{
    if (!makeDirectories(parent(path)))
        return false;

    std::ofstream output(path.c_str(), std::ios::binary | std::ios::trunc);
    if (!output.good())
        return false;
    if (length > 0)
        output.write(static_cast<const char*>(data), static_cast<std::streamsize>(length));
    output.flush();
    return output.good();
}

} // namespace PlatformStorage

#endif // PS2_PLATFORM || WII_PLATFORM
