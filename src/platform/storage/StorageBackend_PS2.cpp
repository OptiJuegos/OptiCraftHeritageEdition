#include "platform/Storage.h"
#include "ps2/storage/save/Ps2SaveFileSystem.h"

namespace PlatformStorage
{
bool exists(const std::string& path) { return Ps2SaveFileSystem::exists(path); }
bool mkdirs(const std::string& path) { return Ps2SaveFileSystem::mkdirs(path); }
bool removeFile(const std::string& path) { return Ps2SaveFileSystem::removeFile(path); }
bool renameFile(const std::string& from, const std::string& to)
{
    (void)from;
    (void)to;
    return false;
}
bool supportsAtomicRename() { return false; }
bool supportsSessionLocks() { return false; }
bool readFile(const std::string& path, std::vector<unsigned char>& out) { return Ps2SaveFileSystem::readFile(path, out); }
bool writeFile(const std::string& path, const void* data, std::size_t length) { return Ps2SaveFileSystem::writeFile(path, data, length); }
bool pathIsDirectory(const std::string& path) { return Ps2SaveFileSystem::isDirectory(path); }
bool listPathEntries(const std::string& path, std::vector<std::string>& out) { return Ps2SaveFileSystem::listDirs(path, out); }
bool listDirs(const std::string& path, std::vector<std::string>& out) { return Ps2SaveFileSystem::listDirs(path, out); }
}
