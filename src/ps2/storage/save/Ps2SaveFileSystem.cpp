#ifdef PS2_PLATFORM

#include "ps2/storage/save/Ps2SaveFileSystem.h"
#include "ps2/storage/save/Ps2MemoryCardFileSystem.h"
#include "platform/storage/PathUtils.h"
#include "platform/storage/PosixFileSystem.h"

namespace Ps2SaveFileSystem
{

bool isMemoryCardPath(const std::string& path)
{
    return Ps2MemoryCardFileSystem::handles(path);
}

bool isDisabledPath(const std::string& path)
{
    const std::string normalized = PlatformStorage::normalizeSlashes(path);
    // Treat the disabled backend as a device prefix, not as a filesystem path.
    // Older callers can concatenate a world name directly ("nosave:World")
    // while newer ones insert a slash ("nosave:/World"). Neither form must
    // ever fall through to newlib, where it would become an unknown device.
    return normalized.compare(0, 7, "nosave:") == 0;
}

bool mkdirs(const std::string& path)
{
    const std::string normalized = PlatformStorage::normalizeSlashes(path);
    if (isDisabledPath(normalized))
        return true;
    if (isMemoryCardPath(normalized))
        return Ps2MemoryCardFileSystem::mkdirs(normalized);
    return PlatformStorage::makeDirectories(normalized);
}

bool writeFile(const std::string& path, const void* data, std::size_t length)
{
    const std::string normalized = PlatformStorage::normalizeSlashes(path);
    if (isDisabledPath(normalized))
        return true;
    if (isMemoryCardPath(normalized))
        return Ps2MemoryCardFileSystem::writeFile(normalized, data, length);
    return PlatformStorage::posixWriteFile(normalized, data, length);
}

bool readFile(const std::string& path, std::vector<unsigned char>& out)
{
    const std::string normalized = PlatformStorage::normalizeSlashes(path);
    if (isDisabledPath(normalized))
    {
        out.clear();
        return false;
    }
    if (isMemoryCardPath(normalized))
        return Ps2MemoryCardFileSystem::readFile(normalized, out);
    return PlatformStorage::posixReadFile(normalized, out);
}

bool exists(const std::string& path)
{
    const std::string normalized = PlatformStorage::normalizeSlashes(path);
    if (isDisabledPath(normalized))
        return false;
    if (isMemoryCardPath(normalized))
        return Ps2MemoryCardFileSystem::exists(normalized);
    return PlatformStorage::posixExists(normalized);
}

bool removeFile(const std::string& path)
{
    const std::string normalized = PlatformStorage::normalizeSlashes(path);
    if (isDisabledPath(normalized))
        return true;
    if (isMemoryCardPath(normalized))
        return Ps2MemoryCardFileSystem::removeFile(normalized);
    return PlatformStorage::removePath(normalized);
}

bool isDirectory(const std::string& path)
{
    const std::string normalized = PlatformStorage::normalizeSlashes(path);
    if (isDisabledPath(normalized))
        return false;
    return PlatformStorage::isDirectory(normalized);
}

bool listDirs(const std::string& path, std::vector<std::string>& out)
{
    const std::string normalized = PlatformStorage::normalizeSlashes(path);
    if (isDisabledPath(normalized))
    {
        out.clear();
        return true;
    }
    if (isMemoryCardPath(normalized))
        return Ps2MemoryCardFileSystem::listEntries(normalized, out);
    return PlatformStorage::listEntries(normalized, out);
}

} // namespace Ps2SaveFileSystem

#endif // PS2_PLATFORM
