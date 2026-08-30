#include "platform/Log.h"
#include "RegionFileCache.h"
#include "java/Arithmetic.h"

#include "platform/PlatformConfig.h"
#include "RegionFile.h"
#include "RegionFileChunkBuffer.h"
#include <sstream>
#include <fstream>
#include <streambuf>
#include <cstdio>
#include <cstdlib>
#include <set>

#ifdef _WIN32
#include <direct.h>
#define MKDIR(p) _mkdir(p)
#else
#include <sys/stat.h>
#define MKDIR(p) mkdir(p, 0755)
#endif

std::map<std::string, std::shared_ptr<RegionFile>> RegionFileCache::cache;
std::mutex RegionFileCache::cacheMutex;

// Directory creation is process-wide just like the open-region cache. Keeping
// this at namespace scope lets clearCache() release world-name strings when a
// save is closed instead of retaining one entry for every world visited during
// the entire session.
static std::set<std::string> s_createdRegionDirs;

#if PLATFORM_BOUNDED_WORLD
// Bound how many RegionFile objects stay open at once, and evict the least
// recently used one (closing/flushing it) once the cache is full, instead of
// letting it grow with every distinct 32x32 chunk area the player has ever
// visited in the session. Two different reasons, one mechanism:
//
//   PS2  each open RegionFile buffers the WHOLE .mcr in RAM (the Memory Card /
//        USB backend has no partial-file access), so an unbounded cache is a
//        slow OOM by itself. 4 covers a region corner.
//   Wii  RegionFile uses a real std::fstream here (see RegionFile.cpp), so an
//        individual open region is cheap -- but every open std::fstream is a
//        libfat file descriptor, and that table is small and shared with
//        everything else the game has open (assets, the save's level.dat,
//        etc). Without this, walking the ~1,600+ blocks it takes to cross one
//        more region boundary a few times in a session slowly exhausts libfat
//        handles rather than heap, which fails in whatever way fopen() fails
//        on a full descriptor table -- not the "Out of memory!" screen, so it
//        would have looked like an unrelated crash. 16 is a conservative cap
//        given the small size of the object itself; raise it if the log below
//        never fires in normal play.
#  if PLATFORM_PS2
static const size_t MAX_OPEN_REGIONS = 4;
#  else
static const size_t MAX_OPEN_REGIONS = 16;
#  endif
static std::vector<std::string> s_regionLru; // front = least recently used

static void touchRegionLru(const std::string &path)
{
    for (auto it = s_regionLru.begin(); it != s_regionLru.end(); ++it)
    {
        if (*it == path)
        {
            s_regionLru.erase(it);
            break;
        }
    }
    s_regionLru.push_back(path);
}
#endif

// Build the path to the .mcr file for chunk (chunkX, chunkZ)
static std::string regionPath(const std::string &worldDir, int_t chunkX, int_t chunkZ, RegionFileCache::Format format)
{
    std::string regionDir = worldDir + "/region";

    // Do not call mkdir for every chunk. On PS2 Memory Card this is very slow
    // and it also floods the emulator log. Try each region directory once.
#ifndef PS2_PLATFORM
    if (s_createdRegionDirs.insert(regionDir).second)
        MKDIR(regionDir.c_str());
#else
    (void)s_createdRegionDirs;
#endif

    std::ostringstream oss;
    oss << regionDir << "/r." << (JavaArithmetic::intShr(chunkX, 5)) << "." << (JavaArithmetic::intShr(chunkZ, 5))
        << (format == RegionFileCache::Format::Anvil ? ".mca" : ".mcr");
    return oss.str();
}

RegionFile* RegionFileCache::getRegionFile(const std::string &worldDir, int_t chunkX, int_t chunkZ)
{
    // Legacy raw-pointer accessor. Runtime chunk I/O uses acquireRegionFile()
    // so an LRU eviction cannot destroy a region while another thread is
    // reading it.
    return acquireRegionFile(worldDir, chunkX, chunkZ).get();
}

std::shared_ptr<RegionFile> RegionFileCache::acquireRegionFile(const std::string &worldDir,
                                                               int_t chunkX, int_t chunkZ,
                                                               Format format)
{
    std::lock_guard<std::mutex> guard(cacheMutex);
    const std::string path = regionPath(worldDir, chunkX, chunkZ, format);

    auto it = cache.find(path);
    if (it != cache.end())
    {
#if PLATFORM_BOUNDED_WORLD
        touchRegionLru(path);
#endif
        return it->second;
    }

#if PLATFORM_BOUNDED_WORLD
    // A worker can retain a shared reference while it inflates a chunk. Never
    // evict that RegionFile out from under the operation; choose another LRU
    // entry instead. With one Wii chunk worker there is normally at most one
    // temporarily pinned region.
    size_t attempts = s_regionLru.size();
    while (!s_regionLru.empty() && cache.size() >= MAX_OPEN_REGIONS && attempts-- > 0)
    {
        const std::string victim = s_regionLru.front();
        s_regionLru.erase(s_regionLru.begin());
        auto vit = cache.find(victim);
        if (vit == cache.end())
            continue;

        if (vit->second.use_count() > 1)
        {
            s_regionLru.push_back(victim);
            continue;
        }

        MC_LOG_INFO("chunk", "[region cache] evict %s (open=%u)\n",
               victim.c_str(), (unsigned)cache.size());
        vit->second->close();
        cache.erase(vit);
    }
#else
    if ((int_t)cache.size() >= 256)
    {
        for (auto &pair : cache)
            pair.second->close();
        cache.clear();
        s_createdRegionDirs.clear();
    }
#endif

    std::shared_ptr<RegionFile> rf = std::make_shared<RegionFile>(path);
    cache[path] = rf;
#if PLATFORM_BOUNDED_WORLD
    touchRegionLru(path);
#endif
    return rf;
}

void RegionFileCache::clearCache()
{
    std::lock_guard<std::mutex> guard(cacheMutex);
    for (auto &pair : cache)
        pair.second->close();
    cache.clear();
    s_createdRegionDirs.clear();
#if PLATFORM_BOUNDED_WORLD
    s_regionLru.clear();
#endif
}

int_t RegionFileCache::getSizeDelta(const std::string &worldDir, int_t chunkX, int_t chunkZ)
{
    return acquireRegionFile(worldDir, chunkX, chunkZ)->getSizeDelta();
}

namespace
{
    // istream that owns the inflated chunk bytes directly. The old path copied
    // them twice more (vector -> string -> istringstream); at ~85KB per chunk
    // those transient copies were real pressure on the PS2's 32MB heap.
    class ByteVecStreamBuf : public std::streambuf
    {
    public:
        explicit ByteVecStreamBuf(std::vector<byte_t> &&bytes) : data(std::move(bytes))
        {
            char *p = reinterpret_cast<char*>(data.data());
            setg(p, p, p + data.size());
        }
    private:
        std::vector<byte_t> data;
    };

    class ByteVecInputStream : public std::istream
    {
    public:
        explicit ByteVecInputStream(std::vector<byte_t> &&bytes)
            : std::istream(nullptr), buf(std::move(bytes))
        {
            rdbuf(&buf); // also clears the badbit from the null ctor
        }
    private:
        ByteVecStreamBuf buf;
    };
}

// Returns an istream with the decompressed chunk data, or nullptr
std::istream* RegionFileCache::getChunkInputStream(const std::string &worldDir, int_t chunkX, int_t chunkZ)
{
    // On console the RegionFile is backed by an in-memory buffer loaded from the
    // Memory Card / USB, so reading a saved chunk is just a buffer decompress.
    std::shared_ptr<RegionFile> rf = acquireRegionFile(worldDir, chunkX, chunkZ);
    std::vector<byte_t> data;
    if (!rf->getChunkData(chunkX & 0x1f, chunkZ & 0x1f, data))
        return nullptr;

    return new ByteVecInputStream(std::move(data));
}

// Returns an ostream that, when destroyed/closed, compresses and writes via RegionFile::write
// We implement this via RegionFileChunkBuffer's close() method. We wrap it with a stringstream
// and flush on close by using a helper.
std::ostream* RegionFileCache::getChunkOutputStream(const std::string &worldDir, int_t chunkX, int_t chunkZ)
{
#ifdef PS2_PLATFORM
    (void)worldDir;
    (void)chunkX;
    (void)chunkZ;
    return new std::ostringstream(std::ios::binary);
#endif

    // We use a stringstream as write buffer; caller calls close() on the RegionFileChunkBuffer wrapper.
    // Since C++ doesn't have Java's DataOutputStream wrapping, we return an ostringstream.
    // The McRegionChunkLoader calls dataoutputstream.close() which we intercept via the RegionFileChunkBuffer.
    // We return a std::ostringstream here; McRegionChunkLoader must explicitly flush to RegionFile.
    // This is handled in McRegionChunkLoader::saveChunk.
    return new std::ostringstream(std::ios::binary);
}
