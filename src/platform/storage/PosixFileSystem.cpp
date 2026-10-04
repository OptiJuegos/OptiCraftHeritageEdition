#if defined(PS2_PLATFORM) || defined(WII_PLATFORM) || defined(CTR_PLATFORM)

#include "platform/storage/PosixFileSystem.h"
#include "platform/storage/PathUtils.h"

#include <cstdio>
#include <cstring>

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
    if (::stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode))
        return true;

    // stat()'s S_ISDIR bit is unreliable through some IOP filesystem drivers
    // (observed with bdmfs_fatfs on PS2 USB mass storage: every directory
    // reports as a regular file) even though the same path opens fine as a
    // directory. opendir() succeeding is authoritative -- a regular file can
    // never be opened as one -- so fall back to it when stat() says no.
    DIR* dir = ::opendir(path.c_str());
    if (!dir)
        return false;
    ::closedir(dir);
    return true;
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
    // ::stat() alone is not a reliable size query on the PS2 cdrom0: IOP
    // driver -- it can report failure for a path that opens and reads back
    // fine through fopen(). fopen()+fseek()/ftell() is the same mechanism
    // Ps2Storage::readWholeFile() already uses as ITS primary path for every
    // other successful disc read (that function only falls back to this one
    // for a driver that cannot seek at all), so use it here first too, and
    // keep stat() only as the last resort.
    FILE* file = std::fopen(path.c_str(), "rb");
    if (file)
    {
        std::int64_t size = -1;
        if (std::fseek(file, 0, SEEK_END) == 0)
        {
            const long position = std::ftell(file);
            if (position >= 0)
                size = static_cast<std::int64_t>(position);
        }
        std::fclose(file);
        if (size >= 0)
            return size;
    }

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
    FILE* input = std::fopen(path.c_str(), "rb");
    if (input == nullptr)
        return false;

    unsigned char buffer[16384];
    for (;;)
    {
        const std::size_t count = std::fread(buffer, 1, sizeof(buffer), input);
        if (count > 0)
            out.insert(out.end(), buffer, buffer + count);
        if (count < sizeof(buffer))
            break;
    }

    const bool ok = std::ferror(input) == 0;
    std::fclose(input);
    if (!ok)
        out.clear();
    return ok;
}

bool posixWriteFile(const std::string& path, const void* data, std::size_t length)
{
    if (!makeDirectories(parent(path)))
        return false;

    FILE* output = std::fopen(path.c_str(), "wb");
    if (output == nullptr)
        return false;

    const bool wroteAll = length == 0 || std::fwrite(data, 1, length, output) == length;
    const bool flushed = std::fflush(output) == 0;
    const bool closed = std::fclose(output) == 0;
    return wroteAll && flushed && closed;
}

bool posixAppendFile(const std::string& path, const void* data, std::size_t length)
{
    const std::string parentDir = parent(path);
    if (!parentDir.empty() && !makeDirectories(parentDir))
        return false;

    FILE* output = std::fopen(path.c_str(), "ab");
    if (output == nullptr)
        return false;

    const bool wroteAll = length == 0 || std::fwrite(data, 1, length, output) == length;
    const bool flushed = std::fflush(output) == 0;
    const bool closed = std::fclose(output) == 0;
    return wroteAll && flushed && closed;
}

} // namespace PlatformStorage

#endif // PS2_PLATFORM || WII_PLATFORM
