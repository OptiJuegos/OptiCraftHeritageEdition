#include "platform/Storage.h"
#include "platform/storage/PathUtils.h"
#include "platform/storage/PosixFileSystem.h"

namespace PlatformStorage
{
bool exists(const std::string& path) { return posixExists(path); }
bool mkdirs(const std::string& path) { return makeDirectories(path); }
bool removeFile(const std::string& path) { return removePath(path); }
bool renameFile(const std::string& from, const std::string& to) { return renamePath(from, to); }
bool supportsAtomicRename() { return true; }
bool supportsSessionLocks() { return true; }
bool readFile(const std::string& path, std::vector<unsigned char>& out) { return posixReadFile(path, out); }
bool writeFile(const std::string& path, const void* data, std::size_t length) { return posixWriteFile(path, data, length); }
bool pathIsDirectory(const std::string& path) { return isDirectory(path); }
bool listPathEntries(const std::string& path, std::vector<std::string>& out) { return listEntries(path, out); }

bool listDirs(const std::string& path, std::vector<std::string>& out)
{
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
