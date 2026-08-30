#include "AnvilSaveConverter.h"

#include <algorithm>
#include <memory>
#include <sstream>

#include "AnvilConverterData.h"
#include "AnvilSaveHandler.h"
#include "BiomeGenBase.h"
#include "ChunkLoader.h"
#include "CompressedStreamTools.h"
#include "IProgressUpdate.h"
#include "NBTTagCompound.h"
#include "RegionFile.h"
#include "RegionFileCache.h"
#include "SaveFormatComparator.h"
#include "WorldChunkManager.h"
#include "WorldChunkManagerHell.h"
#include "WorldInfo.h"
#include "WorldType.h"
#include "java/Arithmetic.h"
#include "platform/Log.h"
#include "platform/Storage.h"

namespace
{
bool hasSuffix(const std::string &value, const std::string &suffix)
{
    return value.size() >= suffix.size() &&
           value.compare(value.size() - suffix.size(), suffix.size(), suffix) == 0;
}

std::string replaceRegionExtension(const std::string &path)
{
    if (!hasSuffix(path, ".mcr"))
        return path + ".mca";
    return path.substr(0, path.size() - 4) + ".mca";
}
}

AnvilSaveConverter::AnvilSaveConverter(const std::string &baseDir)
    : SaveFormatOld(baseDir)
{
}

jstring AnvilSaveConverter::getSaveFormatName()
{
    return "Anvil";
}

std::vector<SaveFormatComparator *> AnvilSaveConverter::getSaveList()
{
    std::vector<SaveFormatComparator *> result;
    std::vector<std::string> directories;
    if (!PlatformStorage::listDirs(baseDirectory, directories))
        return result;

    for (const std::string &name : directories)
    {
        std::unique_ptr<WorldInfo> info(getWorldInfoUTF8(name));
        if (info == nullptr)
            continue;
        const int_t saveVersion = info->getSaveVersion();
        if (saveVersion != 19132 && saveVersion != 19133)
            continue;

        std::string displayName = info->getWorldName();
        if (displayName.empty())
            displayName = name;

        result.push_back(new SaveFormatComparator(name, displayName,
                                                   info->getLastTimePlayed(), 0,
                                                   info->getGameType(),
                                                   saveVersion != getCurrentSaveVersion(),
                                                   info->isHardcoreModeEnabled()));
    }

    return result;
}

int_t AnvilSaveConverter::getCurrentSaveVersion() const
{
    return 19133;
}

void AnvilSaveConverter::flushCache()
{
    RegionFileCache::clearCache();
}

ISaveHandler *AnvilSaveConverter::getSaveLoader(const jstring &s, bool flag)
{
    return new AnvilSaveHandler(baseDirectory, s, flag);
}

bool AnvilSaveConverter::isOldMapFormat(const jstring &s)
{
    std::unique_ptr<WorldInfo> info(getWorldInfoUTF8(s));
    return info != nullptr && info->getSaveVersion() != getCurrentSaveVersion();
}

bool AnvilSaveConverter::convertMapFormat(const jstring &s, IProgressUpdate *progress)
{
    if (progress == nullptr)
        return false;

    progress->setLoadingProgress(0);
    const std::string worldName = s;
    const std::string worldDir = PlatformStorage::join(baseDirectory, worldName);
    const std::string netherDir = PlatformStorage::join(worldDir, "DIM-1");
    const std::string endDir = PlatformStorage::join(worldDir, "DIM1");

    std::vector<std::string> overworldRegions;
    std::vector<std::string> netherRegions;
    std::vector<std::string> endRegions;
    scanRegionFiles(worldDir, overworldRegions);
    if (PlatformStorage::pathIsDirectory(netherDir))
        scanRegionFiles(netherDir, netherRegions);
    if (PlatformStorage::pathIsDirectory(endDir))
        scanRegionFiles(endDir, endRegions);

    const int_t totalRegions = (int_t)(overworldRegions.size() + netherRegions.size() + endRegions.size());
    MC_LOG_INFO("save", "Anvil conversion: %d region files for '%s'\n", totalRegions, worldName.c_str());

    std::unique_ptr<WorldInfo> info(getWorldInfoUTF8(worldName));
    if (info == nullptr)
        return false;

    WorldType::initialize();
    std::unique_ptr<WorldChunkManager> overworldManager;
    if (info->getTerrainType() == WorldType::FLAT)
        overworldManager.reset(new WorldChunkManagerHell(BiomeGenBase::plains, 0.5f, 0.5f));
    else
        overworldManager.reset(new WorldChunkManager(info->getSeed(), info->getTerrainType()));
    std::unique_ptr<WorldChunkManager> netherManager(new WorldChunkManagerHell(BiomeGenBase::hell, 1.0f, 0.0f));
    std::unique_ptr<WorldChunkManager> endManager(new WorldChunkManagerHell(BiomeGenBase::sky, 0.5f, 0.0f));

    int_t completed = 0;
    convertRegionFiles(overworldRegions, overworldManager.get(), completed, totalRegions, progress);
    completed += (int_t)overworldRegions.size();
    convertRegionFiles(netherRegions, netherManager.get(), completed, totalRegions, progress);
    completed += (int_t)netherRegions.size();
    convertRegionFiles(endRegions, endManager.get(), completed, totalRegions, progress);

    info->setSaveVersion(getCurrentSaveVersion());
    if (info->getTerrainType() == WorldType::DEFAULT_1_1)
        info->setTerrainType(WorldType::DEFAULT);

    backupLevelDat(worldName);
    std::unique_ptr<ISaveHandler> handler(getSaveLoader(worldName, false));
    handler->saveWorldInfo(info.get());
    RegionFileCache::clearCache();
    progress->setLoadingProgress(100);
    return true;
}

void AnvilSaveConverter::backupLevelDat(const std::string &worldName)
{
    const std::string worldDir = PlatformStorage::join(baseDirectory, worldName);
    const std::string source = PlatformStorage::join(worldDir, "level.dat");
    const std::string destination = PlatformStorage::join(worldDir, "level.dat_mcr");
    if (!PlatformStorage::exists(source))
    {
        MC_LOG_WARN("save", "Unable to create level.dat_mcr backup: level.dat is missing\n");
        return;
    }
    if (PlatformStorage::exists(destination))
    {
        MC_LOG_WARN("save", "Keeping existing level.dat_mcr backup\n");
        return;
    }
    if (!PlatformStorage::renameFile(source, destination))
        MC_LOG_WARN("save", "Unable to create level.dat_mcr backup\n");
}

void AnvilSaveConverter::scanRegionFiles(const std::string &dimensionDir,
                                         std::vector<std::string> &files) const
{
    const std::string regionDir = PlatformStorage::join(dimensionDir, "region");
    std::vector<std::string> entries;
    if (!PlatformStorage::listPathEntries(regionDir, entries))
        return;

    for (const std::string &entry : entries)
    {
        const std::string path = PlatformStorage::join(regionDir, entry);
        if (!PlatformStorage::pathIsDirectory(path) && hasSuffix(entry, ".mcr"))
            files.push_back(path);
    }
    std::sort(files.begin(), files.end());
}

void AnvilSaveConverter::convertRegionFiles(const std::vector<std::string> &files,
                                             WorldChunkManager *worldChunkManager,
                                             int_t completedRegions, int_t totalRegions,
                                             IProgressUpdate *progress)
{
    for (std::size_t i = 0; i < files.size(); ++i)
    {
        convertRegionFile(files[i], worldChunkManager,
                          completedRegions + (int_t)i, totalRegions, progress);
        if (totalRegions > 0)
        {
            const int_t value = JavaArithmetic::longToInt(JavaArithmetic::roundDouble(
                100.0 * (double)(completedRegions + (int_t)i + 1) / (double)totalRegions));
            progress->setLoadingProgress(value);
        }
    }
}

void AnvilSaveConverter::convertRegionFile(const std::string &inputPath,
                                            WorldChunkManager *worldChunkManager,
                                            int_t completedRegions, int_t totalRegions,
                                            IProgressUpdate *progress)
{
    RegionFile input(inputPath);
    RegionFile output(replaceRegionExtension(inputPath));
    std::vector<byte_t> raw;

    for (int_t localX = 0; localX < 32; ++localX)
    {
        for (int_t localZ = 0; localZ < 32; ++localZ)
        {
            if (!input.hasChunk(localX, localZ) || output.hasChunk(localX, localZ))
                continue;

            RegionFile::ReadStatus status = RegionFile::ReadStatus::Missing;
            if (!input.getChunkData(localX, localZ, raw, &status) || raw.empty())
            {
                MC_LOG_WARN("save", "Unable to read McRegion chunk %d,%d from %s\n",
                            localX, localZ, inputPath.c_str());
                continue;
            }

            try
            {
                const std::string blob(reinterpret_cast<const char *>(raw.data()), raw.size());
                std::istringstream in(blob, std::ios::binary);
                std::unique_ptr<NBTTagCompound> root(CompressedStreamTools::readCompound(in));
                if (root == nullptr || !root->hasKey("Level"))
                    continue;

                NBTTagCompound *legacyLevel = root->getCompoundTag("Level");
                std::unique_ptr<AnvilConverterData> data(ChunkLoader::load(legacyLevel));
                if (data == nullptr)
                    continue;

                std::unique_ptr<NBTTagCompound> convertedRoot(new NBTTagCompound());
                NBTTagCompound *convertedLevel = new NBTTagCompound();
                convertedRoot->setTag("Level", convertedLevel);
                ChunkLoader::convertToAnvilFormat(data.get(), convertedLevel, worldChunkManager);

                std::ostringstream out(std::ios::binary);
                CompressedStreamTools::writeCompound(convertedRoot.get(), out);
                const std::string convertedBytes = out.str();
                if (!convertedBytes.empty())
                    output.write(localX, localZ,
                                 reinterpret_cast<const byte_t *>(convertedBytes.data()),
                                 (int_t)convertedBytes.size());
            }
            catch (const std::exception &e)
            {
                MC_LOG_ERROR("save", "Failed converting %s chunk %d,%d: %s\n",
                             inputPath.c_str(), localX, localZ, e.what());
            }
            catch (...)
            {
                MC_LOG_ERROR("save", "Failed converting %s chunk %d,%d\n",
                             inputPath.c_str(), localX, localZ);
            }
        }

        if (progress != nullptr && totalRegions > 0)
        {
            const double doneChunks = (double)(completedRegions * 1024 + (localX + 1) * 32);
            const double totalChunks = (double)totalRegions * 1024.0;
            progress->setLoadingProgress(JavaArithmetic::longToInt(
                JavaArithmetic::roundDouble(100.0 * doneChunks / totalChunks)));
        }
    }

    input.close();
    output.close();
}
