#include "SaveFormatOld.h"
#include "platform/Log.h"

#include "SaveFormatComparator.h"
#include "SaveHandler.h"
#include "CompressedStreamTools.h"
#include "NBTTagCompound.h"
#include "WorldInfo.h"
#include "java/File.h"
#include "java/String.h"
#include "platform/PlatformConfig.h"
#include "platform/Storage.h"
#include "RegionFileCache.h"

#include <memory>
#include <sstream>
#include <vector>

namespace
{
bool readCompoundFile(const std::string& path, std::unique_ptr<NBTTagCompound>& out)
{
    std::vector<unsigned char> bytes;
    if (!PlatformStorage::readFile(path, bytes) || bytes.empty())
        return false;
    try
    {
        const std::string blob(reinterpret_cast<const char*>(bytes.data()), bytes.size());
        std::istringstream stream(blob, std::ios::binary);
        out.reset(CompressedStreamTools::readGzippedCompound(stream));
        return out != nullptr;
    }
    catch (...)
    {
        out.reset();
        return false;
    }
}

bool writeCompoundFile(const std::string& path, NBTTagCompound* root)
{
    if (root == nullptr)
        return false;
    try
    {
        std::ostringstream stream(std::ios::binary);
        CompressedStreamTools::writeGzippedCompoundToOutputStream(root, stream);
        const std::string blob = stream.str();
        return PlatformStorage::writeFile(path, blob.data(), blob.size());
    }
    catch (...)
    {
        return false;
    }
}

void deletePathRecursively(const std::string& path)
{
    if (!PlatformStorage::exists(path))
        return;

    if (PlatformStorage::pathIsDirectory(path))
    {
        std::vector<std::string> entries;
        if (PlatformStorage::listPathEntries(path, entries))
        {
            for (const std::string& entry : entries)
                deletePathRecursively(PlatformStorage::join(path, entry));
        }
    }

    const bool removed = PlatformStorage::removeFile(path);
    MC_LOG_DEBUG("save", "delete '%s' -> %s\n", path.c_str(), removed ? "OK" : "FAIL");
}
}

SaveFormatOld::SaveFormatOld(const std::string &baseDir)
    : field_22180_a(File::open(baseDir))
    , baseDirectory(baseDir)
{
    PlatformStorage::mkdirs(baseDirectory);
}

SaveFormatOld::~SaveFormatOld()
{
    delete field_22180_a;
}

jstring SaveFormatOld::getSaveFormatName()
{
    return "Old Format";
}

std::vector<SaveFormatComparator *> SaveFormatOld::getSaveList()
{
    std::vector<SaveFormatComparator *> list;
#if PLATFORM_ENUMERATE_SAVE_DIRECTORIES
    std::vector<std::string> dirs;
    PlatformStorage::listDirs(baseDirectory, dirs);
    MC_LOG_DEBUG("save", "getSaveList base='%s' dirs=%u\n", baseDirectory.c_str(), (unsigned)dirs.size());
    for (const std::string &s : dirs)
    {
        std::unique_ptr<WorldInfo> info(getWorldInfoUTF8(s));
        if (info)
            list.push_back(new SaveFormatComparator(s, s, info->getLastTimePlayed(), info->getSizeOnDisk(),
                                                        info->getGameType(), false, info->isHardcoreModeEnabled()));
    }
#else
    // Preserve Beta 1.7.3's desktop slot probing. Consoles that use typed world
    // directory names can opt into PLATFORM_ENUMERATE_SAVE_DIRECTORIES.
    for (int i = 0; i < 5; i++)
    {
        const std::string s = "World" + std::to_string(i + 1);
        std::unique_ptr<WorldInfo> info(getWorldInfoUTF8(s));
        if (info)
            list.push_back(new SaveFormatComparator(s, "", info->getLastTimePlayed(), info->getSizeOnDisk(),
                                                        info->getGameType(), false, info->isHardcoreModeEnabled()));
    }
#endif
    return list;
}

void SaveFormatOld::flushCache()
{
}

WorldInfo* SaveFormatOld::getWorldInfoUTF8(const std::string &s)
{
    const std::string worldDir = PlatformStorage::join(baseDirectory, s);
    for (const char* name : { "level.dat", "level.dat_old" })
    {
        std::unique_ptr<NBTTagCompound> root;
        if (!readCompoundFile(PlatformStorage::join(worldDir, name), root))
            continue;
        if (root->hasKey("Data"))
            return new WorldInfo(root->getCompoundTag("Data"));
    }
    return nullptr;
}

WorldInfo* SaveFormatOld::getWorldInfo(const jstring &s)
{
    return getWorldInfoUTF8(s);
}

void SaveFormatOld::renameWorld(const jstring &s, const jstring &s1)
{
    const std::string path = PlatformStorage::join(PlatformStorage::join(baseDirectory, s), "level.dat");
    std::unique_ptr<NBTTagCompound> root;
    if (!readCompoundFile(path, root) || !root->hasKey("Data"))
        return;

    root->getCompoundTag("Data")->setString("LevelName", s1);
    writeCompoundFile(path, root.get());
}

void SaveFormatOld::deleteWorldDirectory(const jstring &s)
{
    RegionFileCache::clearCache();
    deletePathRecursively(PlatformStorage::join(baseDirectory, s));
}

ISaveHandler* SaveFormatOld::getSaveLoader(const jstring &s, bool flag)
{
    return new SaveHandler(baseDirectory, s, flag);
}

bool SaveFormatOld::isOldMapFormat(const jstring &s)
{
    return false;
}

bool SaveFormatOld::convertMapFormat(const jstring &s, IProgressUpdate *iprogressupdate)
{
    return false;
}
