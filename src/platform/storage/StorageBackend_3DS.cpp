#include "platform/Storage.h"
#include "platform/storage/AssetPak.h"
#include "platform/storage/PakStorage.h"
#include "platform/storage/PathUtils.h"
#include "platform/storage/PosixFileSystem.h"

#include <cstdio>

// "pak://" paths are the mounted assets.pak (read-only; see PakStorage), so a
// world shipped inside it -- the legacy tutorial -- opens through the same
// save-format code as one on the SD card. The 3DS reaches that card through
// libctru's fsdev layer: fsdevMountSdmc() registers the "sdmc" newlib device,
// so every real path here is an "sdmc:/..." POSIX path and the same
// PosixFileSystem calls the Wii uses work unchanged.
namespace PlatformStorage
{
bool exists(const std::string& path)
{
    if (AssetPak::isPakPath(path))
        return PakStorage::exists(path);
    return posixExists(path);
}
bool mkdirs(const std::string& path)
{
    // Nothing to create inside the pak; the directory is there if entries are.
    if (AssetPak::isPakPath(path))
        return PakStorage::isDirectory(path);
    return makeDirectories(path);
}
bool removeFile(const std::string& path) { return removePath(path); }
bool renameFile(const std::string& from, const std::string& to) { return renamePath(from, to); }
bool supportsAtomicRename() { return true; }
// fsdev's sdmc: is a real rename-capable, lock-capable filesystem, same as the
// Wii's libfat-backed one -- unlike the PS2's memory-card layer.
bool supportsSessionLocks() { return true; }
bool readFile(const std::string& path, std::vector<unsigned char>& out)
{
    if (AssetPak::isPakPath(path))
        return PakStorage::readFile(path, out);
    return posixReadFile(path, out);
}
std::int64_t getFileSize(const std::string& path)
{
    if (AssetPak::isPakPath(path))
        return PakStorage::fileSize(path);
    return fileSize(path);
}
bool readFileRange(const std::string& path, std::size_t offset, void* out, std::size_t length)
{
    if (AssetPak::isPakPath(path))
        return PakStorage::readFileRange(path, offset, out, length);
    std::FILE* file = std::fopen(path.c_str(), "rb");
    if (file == nullptr)
        return false;
    const bool ok = std::fseek(file, static_cast<long>(offset), SEEK_SET) == 0 &&
                    std::fread(out, 1, length, file) == length;
    std::fclose(file);
    return ok;
}
bool writeFile(const std::string& path, const void* data, std::size_t length) { return posixWriteFile(path, data, length); }
bool appendFile(const std::string& path, const void* data, std::size_t length) { return posixAppendFile(path, data, length); }
bool pathIsDirectory(const std::string& path)
{
    if (AssetPak::isPakPath(path))
        return PakStorage::isDirectory(path);
    return isDirectory(path);
}
bool listPathEntries(const std::string& path, std::vector<std::string>& out)
{
    if (AssetPak::isPakPath(path))
        return PakStorage::listEntries(path, out);
    return listEntries(path, out);
}

bool listDirs(const std::string& path, std::vector<std::string>& out)
{
    if (AssetPak::isPakPath(path))
        return PakStorage::listDirs(path, out);

    std::vector<std::string> entries;
    if (!listEntries(path, entries))
    {
        out.clear();
        return false;
    }

    out.clear();
    for (const std::string& entry : entries)
    {
        if (isDirectory(join(path, entry)))
            out.push_back(entry);
    }
    return true;
}
}
