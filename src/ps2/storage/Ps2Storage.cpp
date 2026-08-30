#ifdef PS2_PLATFORM

#include "ps2/storage/Ps2Storage.h"
#include "platform/storage/PosixFileSystem.h"

#include <cstdio>
#include <cstdlib>
#include <malloc.h>
#include <string>

namespace
{
    bool s_fileIoReady = false;
}

namespace Ps2Storage
{

void setFileIoReady()
{
    s_fileIoReady = true;
}

bool fileIoReady()
{
    return s_fileIoReady;
}

std::string massRoot()
{
    if (PlatformStorage::directoryAvailable("mass:/") || PlatformStorage::directoryAvailable("mass:"))
        return "mass:/";

    char root[16];
    for (int unit = 0; unit < 10; ++unit)
    {
        std::snprintf(root, sizeof(root), "mass%d:/", unit);
        if (PlatformStorage::directoryAvailable(root))
            return root;
    }
    return std::string();
}

bool massAvailable()
{
    return !massRoot().empty();
}

unsigned char* readWholeFile(const std::string& path,
                             unsigned int* outSize,
                             std::size_t alignment)
{
    if (outSize)
        *outSize = 0;

    FILE* file = std::fopen(path.c_str(), "rb");
    if (!file)
        return nullptr;

    long size = -1;
    bool needsReopen = false;
    if (std::fseek(file, 0, SEEK_END) == 0)
    {
        size = std::ftell(file);
        needsReopen = true;
    }

    // Some read-only device drivers expose stat()/fread() but do not support
    // seeking. Fall back to the filesystem-reported size so optical or other
    // mounted transports can still use the aligned whole-file loader.
    if (size <= 0)
    {
        const std::int64_t reportedSize = PlatformStorage::fileSize(path);
        if (reportedSize > 0 && reportedSize <= 0x7fffffffLL)
            size = static_cast<long>(reportedSize);
        needsReopen = true;
    }

    if (size <= 0)
    {
        std::fclose(file);
        return nullptr;
    }

    if (needsReopen)
    {
        std::fclose(file);
        file = std::fopen(path.c_str(), "rb");
        if (!file)
            return nullptr;
    }

    unsigned char* data = static_cast<unsigned char*>(memalign(alignment, static_cast<std::size_t>(size)));
    if (!data) {
        std::fclose(file);
        return nullptr;
    }

    const std::size_t read = std::fread(data, 1, static_cast<std::size_t>(size), file);
    std::fclose(file);
    if (read != static_cast<std::size_t>(size)) {
        std::free(data);
        return nullptr;
    }

    if (outSize)
        *outSize = static_cast<unsigned int>(size);
    return data;
}

} // namespace Ps2Storage

#endif // PS2_PLATFORM
