#include "SaveHandler.h"
#include "platform/Log.h"

#include "platform/PlatformConfig.h"
#include "platform/Storage.h"
#include "ChunkLoader.h"
#include "McRegionChunkLoader.h"
#include "CompressedStreamTools.h"
#include "NBTTagCompound.h"
#include "WorldInfo.h"
#include "WorldProvider.h"
#include "WorldProviderHell.h"
#include "MinecraftException.h"
#include "java/File.h"
#include "java/IOUtil.h"
#include "java/String.h"

#include <chrono>
#include <memory>
#include <sstream>
#include <vector>

namespace
{
void mkdirs(const std::string& path)
{
    PlatformStorage::mkdirs(path);
}

bool readWholeFile(const std::string& path, std::string& out)
{
    std::vector<unsigned char> bytes;
    if (!PlatformStorage::readFile(path, bytes))
        return false;
    out.assign(reinterpret_cast<const char*>(bytes.data()), bytes.size());
    return true;
}

bool writeWholeFile(const std::string& path, const std::string& bytes)
{
    return PlatformStorage::writeFile(path, bytes.data(), bytes.size());
}

bool serializeWorldInfo(NBTTagCompound* root, std::string& out)
{
    try
    {
        std::ostringstream stream(std::ios::binary);
        CompressedStreamTools::writeGzippedCompoundToOutputStream(root, stream);
        out = stream.str();
        return !out.empty();
    }
    catch (...)
    {
        out.clear();
        return false;
    }
}

bool commitLevelDat(const std::string& saveDirectory, const std::string& bytes)
{
    const std::string current = PlatformStorage::join(saveDirectory, "level.dat");

    // PS2 Memory Card storage deliberately avoids rename. A single whole-file
    // replace is the native operation there; every other backend keeps the
    // vanilla level.dat_new -> level.dat_old -> level.dat commit sequence.
    if (!PlatformStorage::supportsAtomicRename())
        return writeWholeFile(current, bytes);

    const std::string next = PlatformStorage::join(saveDirectory, "level.dat_new");
    const std::string old = PlatformStorage::join(saveDirectory, "level.dat_old");
    if (!writeWholeFile(next, bytes))
        return false;

    if (PlatformStorage::exists(old))
        PlatformStorage::removeFile(old);
    if (PlatformStorage::exists(current) && !PlatformStorage::renameFile(current, old))
        return false;
    if (!PlatformStorage::renameFile(next, current))
        return false;
    if (PlatformStorage::exists(next))
        PlatformStorage::removeFile(next);
    return true;
}
}

SaveHandler::SaveHandler(const std::string &baseDir, const std::string &name, bool createPlayers) :
    saveDirectory(PlatformStorage::join(baseDir, name)),
    saveDirectoryName(name),
    playersDirectory(PlatformStorage::join(PlatformStorage::join(baseDir, name), "players")),
    dataDirectory(PlatformStorage::join(PlatformStorage::join(baseDir, name), "data"))
{
    sessionTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    mkdirs(saveDirectory);
    mkdirs(dataDirectory);
    if (createPlayers)
        mkdirs(playersDirectory);

    writeSessionLock();
}

void SaveHandler::writeSessionLock()
{
    if (!PlatformStorage::supportsSessionLocks())
        return;

    std::ostringstream stream(std::ios::binary);
    IOUtil::writeLong(stream, sessionTime);
    if (!stream.good() || !writeWholeFile(PlatformStorage::join(saveDirectory, "session.lock"), stream.str()))
        throw std::runtime_error("Failed to write session lock, aborting");
}

void SaveHandler::checkSessionLock()
{
    if (!PlatformStorage::supportsSessionLocks())
        return;

    try
    {
        std::string bytes;
        if (!readWholeFile(PlatformStorage::join(saveDirectory, "session.lock"), bytes))
            throw MinecraftException("Failed to check session lock, aborting");
        std::istringstream stream(bytes, std::ios::binary);
        const long_t timestamp = IOUtil::readLong(stream);
        if (!stream.good() && !stream.eof())
            throw MinecraftException("Failed to check session lock, aborting");
        if (timestamp != sessionTime)
            throw MinecraftException("The save is being accessed from another location, aborting");
    }
    catch (const MinecraftException&)
    {
        throw;
    }
    catch (...)
    {
        throw MinecraftException("Failed to check session lock, aborting");
    }
}

IChunkLoader* SaveHandler::getChunkLoader(WorldProvider *worldprovider)
{
#if PLATFORM_BOUNDED_WORLD
    if (dynamic_cast<WorldProviderHell*>(worldprovider))
        return new McRegionChunkLoader(PlatformStorage::join(saveDirectory, "DIM-1"));
    return new McRegionChunkLoader(saveDirectory);
#else
    if (dynamic_cast<WorldProviderHell*>(worldprovider))
    {
        const std::string dim1 = PlatformStorage::join(saveDirectory, "DIM-1");
        mkdirs(dim1);
        return new ChunkLoader(File::open(dim1), true);
    }
    return new ChunkLoader(File::open(saveDirectory), true);
#endif
}

WorldInfo* SaveHandler::loadWorldInfo()
{
    for (const std::string& name : { "level.dat", "level.dat_old" })
    {
        std::string bytes;
        if (!readWholeFile(PlatformStorage::join(saveDirectory, name), bytes) || bytes.empty())
            continue;
        try
        {
            std::istringstream stream(bytes, std::ios::binary);
            std::unique_ptr<NBTTagCompound> root(CompressedStreamTools::readGzippedCompound(stream));
            if (!root || !root->hasKey("Data"))
                continue;
            return new WorldInfo(root->getCompoundTag("Data"));
        }
        catch (...)
        {
        }
    }
    return nullptr;
}

void SaveHandler::saveWorldInfoAndPlayer(WorldInfo *worldinfo, const std::vector<EntityPlayer *> &players)
{
    NBTTagCompound root;
    root.setTag("Data", worldinfo->getNBTTagCompoundWithPlayer(players));

    std::string bytes;
    if (!serializeWorldInfo(&root, bytes))
        return;
    const bool ok = commitLevelDat(saveDirectory, bytes);
    MC_LOG_DEBUG("save", "level.dat (with player) write %s, %u bytes\n",
                 ok ? "OK" : "FAIL", (unsigned)bytes.size());
}

void SaveHandler::saveWorldInfo(WorldInfo *worldinfo)
{
    NBTTagCompound root;
    root.setTag("Data", worldinfo->getNBTTagCompound());

    std::string bytes;
    if (!serializeWorldInfo(&root, bytes))
        return;
    const bool ok = commitLevelDat(saveDirectory, bytes);
    MC_LOG_DEBUG("save", "level.dat write %s, %u bytes\n",
                 ok ? "OK" : "FAIL", (unsigned)bytes.size());
}

std::string SaveHandler::getMapFile(const jstring &s)
{
    return PlatformStorage::join(dataDirectory, s + ".dat");
}

std::string SaveHandler::getSaveDirectoryName() const
{
    return saveDirectoryName;
}
