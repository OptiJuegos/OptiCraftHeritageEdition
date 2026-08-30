#ifdef PS2_PLATFORM

#include "platform/Log.h"
#include "ps2/storage/save/Ps2MemoryCardFileSystem.h"
#include "platform/storage/PathUtils.h"
#include "platform/storage/PosixFileSystem.h"

#include <cstdio>
#include <cstring>
#include <fstream>

#include <fcntl.h>
#include <libmc.h>
#include <unistd.h>

#ifndef MC_O_CREAT_FILE
#define MC_O_CREAT_FILE (O_CREAT | O_RDWR | O_TRUNC)
#endif
#ifndef FIO_O_RDONLY
#define FIO_O_RDONLY O_RDONLY
#endif

namespace
{
    unsigned char s_bounce[16384] __attribute__((aligned(64)));

    int mcWait()
    {
        int command = 0;
        int result = -1;
        mcSync(0, &command, &result);
        return result;
    }

    std::string toCardPath(const std::string& path)
    {
        std::string result = PlatformStorage::normalizeSlashes(path);
        if (PlatformStorage::hasPrefix(result, "mc0:") || PlatformStorage::hasPrefix(result, "mc1:"))
            result.erase(0, 4);
        if (result.empty() || result.front() != '/')
            result.insert(result.begin(), '/');

        const std::size_t separator = result.find('/', 1);
        if (separator != std::string::npos)
            for (std::size_t i = separator + 1; i < result.size(); ++i)
                if (result[i] == '/')
                    result[i] = '_';
        return result;
    }

    bool mkdirCardPath(const std::string& cardPath)
    {
        for (std::size_t i = 1; i <= cardPath.size(); ++i)
        {
            if (i != cardPath.size() && cardPath[i] != '/')
                continue;

            const std::string directory = cardPath.substr(0, i);
            if (directory.empty() || directory == "/")
                continue;
            mcMkDir(0, 0, directory.c_str());
            mcWait();
        }
        return true;
    }

    bool readWithLibMc(const std::string& path, std::vector<unsigned char>& out)
    {
        const std::string cardPath = toCardPath(path);
        const int request = mcOpen(0, 0, cardPath.c_str(), FIO_O_RDONLY);
        if (request < 0)
            return false;
        const int fd = mcWait();
        if (fd < 0)
            return false;

        mcSeek(fd, 0, SEEK_END);
        const int size = mcWait();
        mcSeek(fd, 0, SEEK_SET);
        mcWait();

        const std::size_t readCap = 32u * 1024u * 1024u;
        if (size > 0 && static_cast<std::size_t>(size) <= readCap)
            out.reserve(static_cast<std::size_t>(size));

        for (;;)
        {
            const int requestRead = mcRead(fd, s_bounce, static_cast<int>(sizeof(s_bounce)));
            const int count = requestRead >= 0 ? mcWait() : -1;
            if (count <= 0)
                break;
            out.insert(out.end(), s_bounce, s_bounce + count);
            if (out.size() >= readCap)
                break;
            if (size > 0 && static_cast<int>(out.size()) >= size)
                break;
        }

        mcClose(fd);
        mcWait();
        return !out.empty();
    }

    bool readWithNewlib(const std::string& path, std::vector<unsigned char>& out)
    {
        std::ifstream input(path.c_str(), std::ios::binary);
        if (!input.good())
            return false;

        char buffer[4096];
        while (input.read(buffer, sizeof(buffer)) || input.gcount() > 0)
            out.insert(out.end(), buffer, buffer + input.gcount());
        return !out.empty();
    }
}

namespace Ps2MemoryCardFileSystem
{

bool handles(const std::string& path)
{
    return PlatformStorage::hasPrefix(path, "mc0:") || PlatformStorage::hasPrefix(path, "mc1:");
}

bool mkdirs(const std::string& path)
{
    return mkdirCardPath(toCardPath(path));
}

bool writeFile(const std::string& path, const void* data, std::size_t length)
{
    const std::string cardPath = toCardPath(path);
    mkdirCardPath(PlatformStorage::parent(cardPath));
    mcDelete(0, 0, cardPath.c_str());
    mcWait();

    const int request = mcOpen(0, 0, cardPath.c_str(), MC_O_CREAT_FILE);
    if (request < 0)
        return false;
    const int fd = mcWait();
    if (fd < 0)
        return false;

    bool ok = true;
    const unsigned char* source = static_cast<const unsigned char*>(data);
    std::size_t remaining = length;
    while (remaining > 0)
    {
        const std::size_t count = remaining < sizeof(s_bounce) ? remaining : sizeof(s_bounce);
        std::memcpy(s_bounce, source, count);
        const int requestWrite = mcWrite(fd, s_bounce, static_cast<int>(count));
        const int result = requestWrite >= 0 ? mcWait() : -1;
        if (result != static_cast<int>(count))
        {
            ok = false;
            break;
        }
        source += count;
        remaining -= count;
    }

    mcFlush(fd);
    mcWait();
    mcClose(fd);
    mcWait();
    return ok;
}

bool readFile(const std::string& path, std::vector<unsigned char>& out)
{
    out.clear();
    if (readWithLibMc(path, out))
    {
        MC_LOG_INFO("save", "[PS2] mc_read libmc '%s' bytes=%u\n", path.c_str(), static_cast<unsigned>(out.size()));
        return true;
    }

    out.clear();
    const bool ok = readWithNewlib(path, out);
    MC_LOG_INFO("save", "[PS2] mc_read ifstream '%s' ok=%d bytes=%u\n",
                path.c_str(), static_cast<int>(ok), static_cast<unsigned>(out.size()));
    return ok;
}

bool exists(const std::string& path)
{
    const std::string cardPath = toCardPath(path);
    const int request = mcOpen(0, 0, cardPath.c_str(), FIO_O_RDONLY);
    if (request < 0)
        return false;
    const int fd = mcWait();
    if (fd < 0)
        return false;
    mcClose(fd);
    mcWait();
    return true;
}

bool removeFile(const std::string& path)
{
    const std::string cardPath = toCardPath(path);
    mcDelete(0, 0, cardPath.c_str());
    return mcWait() >= 0;
}

bool listEntries(const std::string& path, std::vector<std::string>& out)
{
    return PlatformStorage::listEntries(PlatformStorage::normalizeSlashes(path), out);
}

} // namespace Ps2MemoryCardFileSystem

#endif // PS2_PLATFORM
