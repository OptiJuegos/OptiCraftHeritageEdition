#include "SaveConverterMcRegion.h"
#include "platform/Log.h"
#include "java/Arithmetic.h"

#include "SaveOldDir.h"
#include "SaveFormatComparator.h"
#include "WorldInfo.h"
#include "RegionFileCache.h"
#include "RegionFile.h"
#include "ChunkFile.h"
#include "ChunkFilePattern.h"
#include "IProgressUpdate.h"
#include "MathHelper.h"
#include "java/String.h"
#include <algorithm>
#include <memory>
#include <fstream>
#include <cstdio>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <zlib.h>
#ifdef PS2_PLATFORM
#include "platform/Storage.h"
#endif

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#define MKDIR(p) _mkdir(p)
#else
#include <sys/stat.h>
#include <dirent.h>
#define MKDIR(p) mkdir(p, 0755)
#endif

static bool isDirectory(const std::string &path)
{
#ifdef _WIN32
    DWORD attr = GetFileAttributesA(path.c_str());
    return attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY);
#else
    struct stat st;
    return stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
#endif
}

static std::vector<std::string> listDirectory(const std::string &path)
{
    std::vector<std::string> entries;
#ifdef _WIN32
    WIN32_FIND_DATAA data;
    HANDLE h = FindFirstFileA((path + "/*").c_str(), &data);
    if (h != INVALID_HANDLE_VALUE)
    {
        do
        {
            std::string name = data.cFileName;
            if (name != "." && name != "..")
                entries.push_back(path + "/" + name);
        } while (FindNextFileA(h, &data));
        FindClose(h);
    }
#else
    DIR *dir = opendir(path.c_str());
    if (dir)
    {
        struct dirent *e;
        while ((e = readdir(dir)))
        {
            std::string name = e->d_name;
            if (name != "." && name != "..")
                entries.push_back(path + "/" + name);
        }
        closedir(dir);
    }
#endif
    return entries;
}

static std::string fileName(const std::string &path)
{
    size_t pos = path.find_last_of("/\\");
    return pos == std::string::npos ? path : path.substr(pos + 1);
}

static std::vector<byte_t> gzipDecompress(const std::vector<byte_t> &compressed)
{
    if (compressed.size() > static_cast<std::size_t>(std::numeric_limits<uInt>::max()))
        throw std::runtime_error("gzip input too large");

    const std::size_t maxOut = static_cast<std::size_t>(std::numeric_limits<int_t>::max());
    std::vector<byte_t> out(64 * 1024);

    z_stream zs{};
    if (inflateInit2(&zs, 16 + MAX_WBITS) != Z_OK)
        throw std::runtime_error("gzip decompress failed");

    zs.next_in = const_cast<Bytef *>(reinterpret_cast<const Bytef *>(compressed.data()));
    zs.avail_in = static_cast<uInt>(compressed.size());

    for (;;)
    {
        const std::size_t produced = static_cast<std::size_t>(zs.total_out);
        if (produced > out.size())
        {
            inflateEnd(&zs);
            throw std::runtime_error("gzip decompress failed");
        }

        zs.next_out = reinterpret_cast<Bytef *>(out.data() + produced);
        zs.avail_out = static_cast<uInt>(out.size() - produced);
        const int status = inflate(&zs, Z_NO_FLUSH);
        if (status == Z_STREAM_END)
            break;

        const bool outFull = (status == Z_OK || status == Z_BUF_ERROR) && zs.avail_out == 0;
        if (!outFull || out.size() >= maxOut)
        {
            inflateEnd(&zs);
            throw std::runtime_error("gzip decompress failed");
        }

        std::size_t next = out.size() <= maxOut / 2 ? out.size() * 2 : maxOut;
        if (next <= out.size())
        {
            inflateEnd(&zs);
            throw std::runtime_error("gzip decompress failed");
        }
        out.resize(next);
    }

    const std::size_t actual = static_cast<std::size_t>(zs.total_out);
    inflateEnd(&zs);
    out.resize(actual);
    return out;
}

SaveConverterMcRegion::SaveConverterMcRegion(const std::string &baseDir) :
    SaveFormatOld(baseDir)
{
}

jstring SaveConverterMcRegion::getSaveFormatName()
{
    return "Scaevolus' McRegion";
}

std::vector<SaveFormatComparator *> SaveConverterMcRegion::getSaveList()
{
    std::vector<SaveFormatComparator *> result;

#ifdef PS2_PLATFORM
    // opendir works on mc0: but stat()/isDirectory() does not, so enumerate world
    // folders through the libmc/USB backend and read level.dat via Ps2SaveFileSystem (the File
    // stream layer is stubbed). Our saves are already region-format, so never flag
    // them for conversion (would run the desktop opendir/stat convert path).
    std::vector<std::string> dirs;
    PlatformStorage::listDirs(baseDirectory, dirs);
    MC_LOG_DEBUG("ps2", "McRegion getSaveList base='%s' dirs=%u\n", baseDirectory.c_str(), (unsigned)dirs.size());
    for (const std::string &name : dirs)
    {
        std::unique_ptr<WorldInfo> info(getWorldInfoUTF8(name));
        MC_LOG_DEBUG("ps2", "  world '%s' level.dat=%s\n", name.c_str(), info ? "OK" : "missing");
        if (!info) continue;
        std::string displayName = info->getWorldName();
        if (displayName.empty()) displayName = name;
        result.push_back(new SaveFormatComparator(name, displayName, info->getLastTimePlayed(),
                                                   info->getSizeOnDisk(), false));
    }
    return result;
#else
    for (const std::string &entry : listDirectory(baseDirectory))
    {
        if (!isDirectory(entry)) continue;
        std::string name = fileName(entry);
        std::unique_ptr<WorldInfo> info(getWorldInfoUTF8(name));
        if (!info) continue;

        bool needsConversion = info->getSaveVersion() != 19132;
        std::string displayName = info->getWorldName();
        if (displayName.empty()) displayName = name;

        result.push_back(new SaveFormatComparator(name, displayName, info->getLastTimePlayed(),
                                                   info->getSizeOnDisk(), needsConversion));
    }
    return result;
#endif
}

void SaveConverterMcRegion::flushCache()
{
    RegionFileCache::clearCache();
}

ISaveHandler* SaveConverterMcRegion::getSaveLoader(const jstring &s, bool flag)
{
    return new SaveOldDir(baseDirectory, s, flag);
}

bool SaveConverterMcRegion::isOldMapFormat(const jstring &s)
{
    std::unique_ptr<WorldInfo> info(getWorldInfoUTF8(s));
    return info != nullptr && info->getSaveVersion() == 0;
}

void SaveConverterMcRegion::scanChunkFiles(const std::string &dir,
    std::vector<ChunkFile> &chunks, std::vector<std::string> &folders)
{
    for (const std::string &sub1 : listDirectory(dir))
    {
        if (!isDirectory(sub1)) continue;
        folders.push_back(sub1);

        for (const std::string &sub2 : listDirectory(sub1))
        {
            if (!isDirectory(sub2)) continue;
            for (const std::string &chunkPath : listDirectory(sub2))
            {
                std::string name = fileName(chunkPath);
                if (ChunkFilePattern::accept(name))
                {
                    File *f = File::open(chunkPath);
                    chunks.push_back(ChunkFile(f));
                }
            }
        }
    }
}

void SaveConverterMcRegion::convertChunkFiles(const std::string &worldDir,
    std::vector<ChunkFile> &chunks, int_t doneCount, int_t totalCount, IProgressUpdate *progress)
{
    std::sort(chunks.begin(), chunks.end());
    std::vector<byte_t> buf(4096);

    for (ChunkFile &cf : chunks)
    {
        int_t cx = cf.getChunkX();
        int_t cz = cf.getChunkZ();

        RegionFile *rf = RegionFileCache::getRegionFile(worldDir, cx, cz);
        if (!rf->hasChunk(cx & 0x1f, cz & 0x1f))
        {
            try
            {
                // Read old gzip-compressed chunk file, decompress, write raw to region
                std::ifstream fin(cf.getFile()->toString(), std::ios::binary);
                std::vector<byte_t> compressed((std::istreambuf_iterator<char>(fin)), {});
                std::vector<byte_t> raw = gzipDecompress(compressed);
                if (raw.size() > static_cast<std::size_t>(std::numeric_limits<int_t>::max()))
                    throw std::runtime_error("decompressed chunk is too large");
                rf->write(cx & 0x1f, cz & 0x1f, raw.data(), static_cast<int_t>(raw.size()));
            }
            catch (...) {}
        }

        doneCount++;
        if (totalCount > 0)
            progress->setLoadingProgress(JavaArithmetic::longToInt(JavaArithmetic::roundDouble(100.0 * doneCount / totalCount)));
    }

    RegionFileCache::clearCache();
}

void SaveConverterMcRegion::deleteFolders(std::vector<std::string> &folders,
    int_t doneCount, int_t totalCount, IProgressUpdate *progress)
{
    for (const std::string &folder : folders)
    {
        for (const std::string &entry : listDirectory(folder))
            ::remove(entry.c_str());
        ::rmdir(folder.c_str());

        doneCount++;
        if (totalCount > 0)
            progress->setLoadingProgress(JavaArithmetic::longToInt(JavaArithmetic::roundDouble(100.0 * doneCount / totalCount)));
    }
}

bool SaveConverterMcRegion::convertMapFormat(const jstring &s, IProgressUpdate *iprogressupdate)
{
    iprogressupdate->setLoadingProgress(0);

    std::string worldName = s;
    std::string worldDir  = baseDirectory + "/" + worldName;
    std::string dim1Dir   = worldDir + "/DIM-1";

    std::vector<ChunkFile> chunks, chunks1;
    std::vector<std::string> folders, folders1;

    MC_LOG_INFO("chunk", "Scanning folders...\n");
    scanChunkFiles(worldDir, chunks, folders);
    if (isDirectory(dim1Dir))
        scanChunkFiles(dim1Dir, chunks1, folders1);

    int_t total = (int_t)(chunks.size() + chunks1.size() + folders.size() + folders1.size());
    MC_LOG_INFO("chunk", "Total conversion count is %d\n", total);

    convertChunkFiles(worldDir,  chunks,  0,                  total, iprogressupdate);
    convertChunkFiles(dim1Dir,   chunks1, (int_t)chunks.size(), total, iprogressupdate);

    std::unique_ptr<WorldInfo> info(getWorldInfoUTF8(worldName));
    if (info)
    {
        info->setSaveVersion(19132);
        std::unique_ptr<ISaveHandler> handler(getSaveLoader(s, false));
        handler->saveWorldInfo(info.get());
    }

    deleteFolders(folders,  (int_t)(chunks.size() + chunks1.size()), total, iprogressupdate);
    deleteFolders(folders1, (int_t)(chunks.size() + chunks1.size() + folders.size()), total, iprogressupdate);

    return true;
}
