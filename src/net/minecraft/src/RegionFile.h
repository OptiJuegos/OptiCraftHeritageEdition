#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <mutex>
#include "java/Type.h"
#include "platform/PlatformConfig.h"

// net.minecraft.src.RegionFile
// Uses zlib for compression (matching Java's DeflaterOutputStream / InflaterInputStream)
class RegionFile
{
public:
    enum class ReadStatus
    {
        Missing,
        Success,
        InvalidCoordinates,
        InvalidSector,
        InvalidLength,
        TruncatedData,
        UnsupportedCompression,
        DecompressionFailed,
        IoError
    };

    explicit RegionFile(const std::string &path);
    ~RegionFile();

    int_t getSizeDelta();
    // Decompress into caller-owned scratch.  Chunk I/O is synchronous, so the
    // loader can retain this capacity across saves/loads instead of allocating
    // and freeing an ~85KB vector for every chunk.
    bool getChunkData(int_t x, int_t z, std::vector<byte_t> &out,
                      ReadStatus *status = nullptr);
    std::vector<byte_t> getChunkDataForWrite(int_t x, int_t z);
    void write(int_t x, int_t z, const byte_t *data, int_t length);
    void close();
    bool hasChunk(int_t x, int_t z);

    // Used only by RegionFileCache
    const std::string filePath;

private:
    bool outOfBounds(int_t x, int_t z);
    int_t getOffset(int_t x, int_t z);
    void setOffset(int_t x, int_t z, int_t value);
    void setTimestamp(int_t x, int_t z, int_t value);
    void writeSector(int_t sectorNum, const byte_t *data, int_t length);

#if PLATFORM_REGION_WHOLE_FILE_BUFFER
    // Storage backends without cheap random-access files keep the region in an
    // in-memory buffer: loaded from storage in the ctor, written back in close().
    // All the seek/read/write logic operates on it unchanged.
    std::stringstream dataFile;
#else
    std::fstream dataFile;
#endif
    int_t offsets[1024];
    int_t timestamps[1024];

    // Runtime validation of the 1024 region-header entries.  An offset can be
    // non-zero yet still be unsafe (out of range or overlapping another chunk).
    // Keep that distinction so a corrupt entry is never read as another chunk.
    bool entryValid[1024];

    std::vector<bool> sectorFree;

    // Owner of each physical 4 KiB sector: chunk header index [0,1023], -1 for
    // genuinely free, -2 for a sector referenced by an invalid header entry.
    // Invalid/recoverable data stays reserved until the header is repaired or
    // the region is reopened after a successful rewrite.
    std::vector<int_t> sectorOwner;
    // Compressed data is needed only while the RegionFile lock is held.  Keep
    // one reusable buffer per open region rather than churning the heap for
    // every read and deflate write.
    std::vector<byte_t> ioScratch;
    int_t sizeDelta;
    // Set once write() modifies the region after open/load. The PS2 close()
    // path skips the slow whole-file Memory Card write-back when still clean
    // (regions the player only read while passing through).
    bool dirty = false;
    std::mutex mtx;
#if PLATFORM_REGION_WHOLE_FILE_BUFFER
    // close() flushes the buffer to storage; guard against the double flush from
    // RegionFileCache::clearCache() (explicit close() followed by delete/dtor).
    bool flushed = false;
#endif

    static const byte_t emptySector[4096];
};
