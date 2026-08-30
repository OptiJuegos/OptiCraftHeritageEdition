#ifdef PS2_PLATFORM

#include "java/File.h"
#include "util/Memory.h"
#include "net/minecraft/src/GameResources.h"
#include "ps2/storage/save/Ps2SaveFileSystem.h"
#include "ps2/storage/save/Ps2SaveStorage.h"
#include "platform/Storage.h"
#include "platform/storage/PathUtils.h"
#include "platform/storage/PosixFileSystem.h"

#include <cstring>
#include <fstream>
#include <memory>
#include <string>
#include <vector>
#include <sstream>


static bool ps2_is_file_like_path(const std::string& p)
{
    size_t pos = p.find_last_of("/\\");
    std::string leaf = (pos == std::string::npos) ? p : p.substr(pos + 1);
    if (leaf.empty())
        return false;
    if (leaf == "session.lock")
        return true;
    static const char* exts[] = { ".dat", ".dat_new", ".dat_old", ".mcr", ".txt", ".lock", ".ps2" };
    for (const char* ext : exts)
    {
        const size_t n = std::strlen(ext);
        if (leaf.size() >= n && leaf.compare(leaf.size() - n, n, ext) == 0)
            return true;
    }
    return false;
}

class File_Impl : public File
{
private:
    std::string u8path;

public:
    explicit File_Impl(const jstring& p)
    {
        u8path = PlatformStorage::normalizeSlashes(p);
        path = u8path;
    }

    bool createNewFile() const override
    {
        // Memory Card files are handled by the dedicated mc backend. USB mass
        // storage is a normal writable filesystem and must keep the Java File
        // contract used by world/session/stat writers.
        if (Ps2SaveFileSystem::isDisabledPath(u8path))
            return true;
        if (Ps2SaveFileSystem::isMemoryCardPath(u8path))
            return true;
        return PlatformStorage::createFile(u8path);
    }

    bool remove() const override
    {
        if (Ps2SaveFileSystem::isDisabledPath(u8path))
            return true;
        if (Ps2SaveFileSystem::isMemoryCardPath(u8path))
            return true;
        return PlatformStorage::removePath(u8path);
    }

    bool renameTo(const File& dest) const override
    {
        if (Ps2SaveFileSystem::isDisabledPath(u8path) || Ps2SaveFileSystem::isDisabledPath(dest.toString()))
            return true;
        if (Ps2SaveFileSystem::isMemoryCardPath(u8path) || Ps2SaveFileSystem::isMemoryCardPath(dest.toString()))
            return true;
        return PlatformStorage::renamePath(u8path, dest.toString());
    }

    bool exists() const override
    {
        if (Ps2SaveFileSystem::isDisabledPath(u8path))
            return false;
        if (Ps2SaveFileSystem::isMemoryCardPath(u8path))
            return isDirectory();
        return PlatformStorage::exists(u8path);
    }

    bool isDirectory() const override
    {
        if (Ps2SaveFileSystem::isDisabledPath(u8path))
            return !ps2_is_file_like_path(u8path);
        if (Ps2SaveFileSystem::isMemoryCardPath(u8path))
            return !ps2_is_file_like_path(u8path);
        return PlatformStorage::isDirectory(u8path);
    }

    bool isFile() const override
    {
        if (Ps2SaveFileSystem::isDisabledPath(u8path))
            return false;
        if (Ps2SaveFileSystem::isMemoryCardPath(u8path))
            return false;
        return PlatformStorage::isFile(u8path);
    }

    long_t lastModified() const override
    {
        if (Ps2SaveFileSystem::isDisabledPath(u8path) || Ps2SaveFileSystem::isMemoryCardPath(u8path))
            return 0;
        return static_cast<long_t>(PlatformStorage::lastModifiedMs(u8path));
    }

    long_t length() const override
    {
        if (Ps2SaveFileSystem::isDisabledPath(u8path) || Ps2SaveFileSystem::isMemoryCardPath(u8path))
            return 0;
        const std::int64_t size = PlatformStorage::fileSize(u8path);
        return size >= 0 ? static_cast<long_t>(size) : 0;
    }

    std::vector<std::unique_ptr<File>> listFiles() const override
    {
        std::vector<std::unique_ptr<File>> files;
        if (Ps2SaveFileSystem::isDisabledPath(u8path) || Ps2SaveFileSystem::isMemoryCardPath(u8path))
            return files;

        std::vector<std::string> entries;
        if (!PlatformStorage::listEntries(u8path, entries))
            return files;

        for (const std::string& entry : entries)
            files.push_back(Util::make_unique<File_Impl>(jstring(PlatformStorage::join(u8path, entry))));

        return files;
    }

    File* getParentFile() const override
    {
        return new File_Impl(PlatformStorage::parent(u8path));
    }

    bool mkdir() const override
    {
        if (Ps2SaveFileSystem::isDisabledPath(u8path))
            return true;
        if (Ps2SaveFileSystem::isMemoryCardPath(u8path))
            return !ps2_is_file_like_path(u8path);
        return PlatformStorage::makeDirectory(u8path);
    }

    std::istream* toStreamIn() const override
    {
        if (Ps2SaveFileSystem::isDisabledPath(u8path) || Ps2SaveFileSystem::isMemoryCardPath(u8path))
            return nullptr;
        auto is = Util::make_unique<std::ifstream>(u8path.c_str(), std::ios::binary);
        if (!is->is_open() || !is->good())
            return nullptr;
        return is.release();
    }

    std::ostream* toStreamOut() const override
    {
        // Never send mc0:/mc1: through newlib streams; those paths use libmc.
        // mass:, however, is an ordinary writable filesystem and is the save
        // backend selected by the Pendrive option in the PS2 prompt.
        if (Ps2SaveFileSystem::isDisabledPath(u8path) || Ps2SaveFileSystem::isMemoryCardPath(u8path))
            return new std::ostringstream(std::ios::binary);

        auto os = Util::make_unique<std::ofstream>(u8path.c_str(), std::ios::binary | std::ios::trunc);
        if (!os->is_open() || !os->good())
            return nullptr;
        return os.release();
    }
};

File* File::open(const jstring& path)
{
    return new File_Impl(path);
}

File* File::open(const File& parent, const jstring& child)
{
    return new File_Impl(jstring(PlatformStorage::join(parent.toString(), child)));
}

File* File::openResourceDirectory()
{
    // Resource roots are resolved by GameResources for both console ports.
    return new File_Impl(jstring(GameResources::getAssetsDir()));
}

File* File::openWorkingDirectory(const jstring& name)
{
    return new File_Impl(jstring(PlatformStorage::join(Ps2SaveStorage::root(), name)));
}

#endif // PS2_PLATFORM
